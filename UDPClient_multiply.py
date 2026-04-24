from socket import * 

# create UDP client socket 
clientSocket = socket(AF_INET, SOCK_DGRAM)

# IP address and port number of server program
serverIP = '127.0.0.1' 
serverPort = 12345

# prompt user to enter comma-separated numbers 
# store in list
numbers = input('Enter list of numbers (i.e., number1, number2, number3): ')

# send the server the list of numbers
clientSocket.sendto(numbers.encode(),(serverIP, serverPort))

#receive message back (could be product or error message)
replyMessage, serverAddress = clientSocket.recvfrom(2048)

#print 
print(replyMessage.decode())

clientSocket.close()