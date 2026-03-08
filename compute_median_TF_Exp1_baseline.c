#include <stdlib.h> 
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>

#define MAX_LINE_LENGTH 1000000
#define GENE_ARRAY_SIZE 164000 
#define NUM_TETRANUCS   256
#define GENE_SIZE       10000
#define DEBUG           1


// Store gene-data here //
struct Genes {
    unsigned char* gene_sequences; // The gene-data
    int* gene_sizes;               // The gene lengths
    int  num_genes;                // The number of genes read in
}; // End Genes //




// Read Genes -------------------------- //
//      Reads in the gene-data from a file.
struct Genes read_genes(FILE* inputFile) {

    // return this
    struct Genes genes;
    genes.gene_sequences = (unsigned char*)malloc((GENE_ARRAY_SIZE * GENE_SIZE) * sizeof(unsigned char)); // array of genes
    genes.gene_sizes = (int*)malloc(GENE_ARRAY_SIZE * sizeof(int)); // array of gene lengths
    genes.num_genes = 0;                                            // number of existing genes

    // Remove the first header
    char line[MAX_LINE_LENGTH] = { 0 };
    fgets(line, MAX_LINE_LENGTH, inputFile);

    // Read in lines from the file while line exists
    int currentGeneIndex = 0;
    while (fgets(line, MAX_LINE_LENGTH, inputFile)) {

        // If the line is empty, exit loop.
        //      This if statment helps avoid errors.
        //      (strcmp returns 0 for equal strings)
        if (strcmp(line, "") == 0) {
            break;
        }

        // If line is a DNA sequence:
        //      Read in the nucleotides in order into an array
        else if (line[0] != '>') {

            int line_len = strlen(line);

            //#pragma parallel for
            for (int i = 0; i < line_len; ++i) {
                char c = line[i];
                if (c == 'A' || c == 'C' || c == 'G' || c == 'T') {
                    genes.gene_sequences[genes.num_genes * GENE_SIZE + currentGeneIndex] = c;  // put letter into gene
                    currentGeneIndex += 1;                                                     // increase currentGene size
                }
            }

        }

        // If line is a header:
        //      Reset for another gene-pass.
        else if (line[0] == '>') {
            // indicate we have another gene to read in
            genes.gene_sizes[genes.num_genes] = currentGeneIndex;
            genes.num_genes += 1;
            currentGeneIndex = 0;
        }
    }

    genes.gene_sizes[genes.num_genes] = currentGeneIndex;
    genes.num_genes += 1;


    // done
    return(genes);

} // End Read Genes //



// Process Tetranucs ------------------------ //
/* Input: A DNA sequence of length N for a gene
    Output: The TF of this gene, which is an integer array of length 256

    For each i between 0 and N-4:
            Get the substring from i to i+3 in the DNA sequence
            This substring is a tetranucleotide
            Convert this tetranucleotide to its array index, idx
            TF[idx]++
*/
void process_tetranucs(struct Genes genes, int* gene_TF, int gene_index) {

    // TODO: process the current gene array (Copy from problem 2)
    int geneSize = genes.gene_sizes[gene_index]; // number of characters in gene (use gene_index on) 
    // go through each character and create a window of 4 characters 
    for (int i=0; i < geneSize - 3; i++){
        int window[4] = {0,0,0,0}; // used to store int values of each character
        
        // store 4 letter substring in each iteration in substring array below
        char substring[4] = {
            // access each character from genes_sequences array, which is 2D array stored in row major order
            genes.gene_sequences[gene_index * GENE_SIZE + (i+0)], 
            genes.gene_sequences[gene_index * GENE_SIZE + (i+1)], 
            genes.gene_sequences[gene_index * GENE_SIZE + (i+2)], 
            genes.gene_sequences[gene_index * GENE_SIZE + (i+3)]
        };

        // convert characters to correct int value and store in window 
        for (int c=0; c < 4; c++) {
            char substring_char = substring[c]; // get character 
            if (substring_char == 'A') {
                window[c] = 0;
            } 
            else if (substring_char == 'C') {
                window[c] = 1;
            }
            else if (substring_char == 'G'){
                window[c] = 2;
            }
            else if (substring_char == 'T') {
                window[c] = 3;
            }
        }

        // compute index of tetranucleotide using window 
        int index = window[0] * 64 + window[1] * 16 + window[2] * 4 + window[3];
        gene_TF[index]++;// update count of this tetranucleotide in gene's TF array

    }

} // End Process Tetranucs //

// based off Brocode Youtube video: https://www.youtube.com/watch?v=Dv4qLJcxus8
void bubbleSort(int* freqs, int num_genes) {
    int temp; 
    // n-1 passes to get elements in correct positions
    for (int i = 0; i < num_genes - 1; i++){
        //compare adjacent elements for each pass
        for (int j = 0; j < num_genes - i - 1; j++){
            if (freqs[j] > freqs[j+1]) {
                temp = freqs[j];
                freqs[j] = freqs[j+1];
                freqs[j+1] = temp;
            }
        }
    }
}



// Find Median ------------------------ //
/* Find the median of an unsorted array */
double find_median(int* freqs, int num_genes) {

    // TODO: sort the frequencies
    bubbleSort(freqs, num_genes); // sort using bubble sort to get around 10 minute run time for serial

    double res = 0; 

    // TODO: median if even
    if (num_genes % 2 == 0){
        double n1 = freqs[(num_genes / 2) - 1];
        double n2 = freqs[(num_genes)/2];
        res = (n1 + n2) / 2.0;
    }

    // TODO median if odd
    else{
        res = (double)freqs[(num_genes / 2)];
    }

    return res; 
} // End Find Median //



// Main Program -------------------- //
//      Processes the tetranucleotides.
int main(int argc, char* argv[]) {
    // Check for console errors
    if (argc != 5) {
        printf("USE LIKE THIS:\ncompute_average_TF_Exp1 input.fna average_TF.csv time.csv num_threads\n");
        exit(-1);
    }

    // Get the input file
    FILE* inputFile = fopen(argv[1], "r");
    if (inputFile == NULL) {
        printf("ERROR: Could not open file %s!\n", argv[1]);
        exit(-2);
    }

    // Get the output file
    FILE* outputFile = fopen(argv[2], "w");
    if (outputFile == NULL) {
        printf("ERROR: Could not open file %s!\n", argv[2]);
        fclose(inputFile);
        exit(-3);
    }

    // Get the time file
    FILE* timeFile = fopen(argv[3], "w");
    if (outputFile == NULL) {
        printf("ERROR: Could not open file %s!\n", argv[3]);
        fclose(inputFile);
        fclose(outputFile);
        exit(-4);
    }

    int thread_count = strtol(argv[4], NULL, 10);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

    // Below is a data structure to help you access the gene-file's data:
    //      access gene like: genes.genes[gene_index*GENE_SIZE + char_index]
    //      access each gene's size like: genes.gene_sizes[gene_index]
    //      access the total number of genes like: genes.num_genes
    struct Genes genes = read_genes(inputFile);

    // Store each gene's TF array here
    int** gene_TF_counts = (int**)malloc(genes.num_genes * sizeof(int*)); // (array of pointers to first int of each gene's TF array)

    // Get the start time
    double start = omp_get_wtime();
    /*  1) Tetranuc computation
            For each gene in the list:
                Compute this gene�s TF
                Add this gene�s TF to the running total TF

    */ // TODO: parallelize the computations for each gene for exp 1 and exp 2.
    # pragma omp parallel for num_threads (thread_count)
    for (int gene_index = 0; gene_index < genes.num_genes; ++gene_index) {

        // Compute this gene's TF
        int* gene_TF = (int*)calloc(NUM_TETRANUCS, sizeof(int)); 
        process_tetranucs(genes, gene_TF, gene_index);

        // Save to 2d array
        gene_TF_counts[gene_index] = gene_TF; 
    }


    // 2) Find the medians (as a double!)
    // TODO: parallelize the computations for each median for exp 2 only
    double* median_TF = (double*)calloc(NUM_TETRANUCS, sizeof(double));
    for (int tet = 0; tet < NUM_TETRANUCS; ++tet) {
        int num_genes = genes.num_genes;
        
        // save all frequencies of TF here
        int* freqs = (int*)calloc(num_genes, sizeof(int)); // stores frequencies of this tetranucleotide in all genes 
        for (int gene = 0; gene < num_genes; ++gene)
            freqs[gene] = gene_TF_counts[gene][tet]; // go down column of 2D array

        // find median frequency
        median_TF[tet] = find_median(freqs, num_genes);
        free(freqs);
    }


    // Get the passed time
    double end = omp_get_wtime();

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //


    // Print the median tetranucs
    for (int i = 0; i < NUM_TETRANUCS; ++i) {
        fprintf(outputFile, "%f", median_TF[i]);
        if (i < NUM_TETRANUCS - 1) fprintf(outputFile, "\n");
    }

    // Print the output time
    double time_passed = end - start;
    fprintf(timeFile, "%f", time_passed);


    // Cleanup
    fclose(timeFile);
    fclose(inputFile);
    fclose(outputFile);

    for (int g = 0; g < genes.num_genes; ++g)
        free(gene_TF_counts[g]);
    free(gene_TF_counts);

    free(median_TF);
    free(genes.gene_sizes);
    free(genes.gene_sequences);

    // done
    return 0;

} // End Main //
