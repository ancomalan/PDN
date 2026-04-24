from socket import * 

# create UDP socket 
serverSocket = socket(AF_INET, SOCK_DGRAM)

# bind socket local IP and port 12345
serverSocket.bind(('127.0.0.1', 12345))
print('The server is ready to receive')

#receive a list of numbers 
string_numbers, clientAddress = serverSocket.recvfrom(2048)

# break string into list of smaller strings based on comma delimiter
string_number_list = string_numbers.decode().split(',')
print ("Received: ", string_number_list)

# check if valid input 
valid = True # flag 
for string in string_number_list: 
    if not string.strip().isdigit(): # strip to remove spaces 
        valid = False


# if valid input, then compute product
if valid: 
    # use list comprehension to convert to int list 
    int_number_list = [int(string) for string in string_number_list]

    # perform the multiplication
    # for each string number in list
    product = 1
    for num in int_number_list: 
        product *= num # convert to int and then perform multiplication

    # convert product back to string for encoding 
    # send the product or error message back to client 
    serverSocket.sendto(str(product).encode(),clientAddress)
else: 
    # send error message back to client 
    errorMessage = 'Invalid input'
    serverSocket.sendto(errorMessage.encode(),clientAddress)


serverSocket.close()