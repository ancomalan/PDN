from socket import * 

# create a TCP welcoming socket
welcomeSocket = socket(AF_INET, SOCK_STREAM)

# bind it to the local IP and port 12345
serverIP = '127.0.0.1'
serverPort  = 12345
serverAddress = (serverIP, serverPort)
welcomeSocket.bind(serverAddress)


# welcome socket listens for incoming TCP requests
# allows up to 1 pending request for connection in the queue
welcomeSocket.listen(1)
print('The server is ready to receive')
while True: 
    # server waits on accept() for incoming requests
    # a new connection socket is created on return 
    connectionSocket, clientAddress = welcomeSocket.accept()
    clientIP, clientPort = clientAddress

    #receive a list of numbers (byte string) from connection socket 
    string_numbers = connectionSocket.recv(2048)

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
        connectionSocket.send(str(product).encode())
    else: 
        # send error message back to client 
        errorMessage = 'Invalid input'
        connectionSocket.send(errorMessage.encode())

    connectionSocket.close()
