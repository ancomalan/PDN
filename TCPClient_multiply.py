from socket import * 

# create TCP socket 
clientSocket = socket(AF_INET, SOCK_STREAM)

# connect to server process using (IP, port number) tuple
serverIP = '127.0.0.1' 
serverPort = 12345
serverAddress = (serverIP, serverPort)
clientSocket.connect(serverAddress)

# prompt user to enter comma-separated numbers and send byte string to client socket
numbers = input('Enter list of numbers (i.e., number1, number2, number3): ')

# send the server the list of numbers
clientSocket.send(numbers.encode())

#receive message (byte string) back (could be product or error message)
replyMessage = clientSocket.recv(2048)

#print 
print(replyMessage.decode())

clientSocket.close()