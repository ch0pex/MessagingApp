import subprocess
import sys
import PySimpleGUI as sg
from enum import Enum
import argparse
import socket
import threading


class client :

    # ******************** TYPES *********************
    # *
    # * @brief Return codes for the protocol methods
    class RC(Enum) :  
        # Esto por algun motivo estaba cambiado a como sale en el enunuciado USER_ERROR valia = 2 y ERROR valia = 1
        # PERO en el enunciado pone que sea al reves, asi que lo he cambiado
        OK = 0
        USER_ERROR = 1
        ERROR = 2

    # ****************** ATTRIBUTES ******************
    _server = None
    _port = -1
    _quit = 0
    _username = None
    _alias = None
    _date = None

    # ******************** METHODS *******************
    # Funcion que decodifica la salida del socket
    @staticmethod
    def recvMessage(sock):
        a = ''
        while True:
            msg = sock.recv(1)
            if (msg == b'\0'):
                break;
            a += msg.decode()
        return(a)
    
    @staticmethod
    def recvFullMessage(sock):
        a = ''
        while True:
            msg = sock.recv(1024)
            if (msg == b'\0'):
                break;
            a += msg.decode()
        return(a)
    # Limpia los campos de registro en caso de que alguno no sea válido
    @staticmethod
    def clearRegisterData():
        client._username = None
        client._alias = None
        client._date= None


    @staticmethod
    def messageListen(sock): 
        sock.listen(5)
        while True:
            conn, addr = sock.accept()
            data = conn.recv(1024)
            if data:
                print(data.decode())
            else:
                break



    # *
    # * @param user - User name to register in the system
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user is already registered
    # * @return ERROR if another error occurred
    
    @staticmethod
    def  register(user, window):
        # Creamos el socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # --INICIO CONEXION--
        server_address = (client._server, client._port)
        try:
            sock.connect(server_address)
        except Exception as e:
            print("Error de conexión con el servidor: ", e)

        # Ponemos los datos a enviar en el formato correcto (añadiendo caracter de fin de linea) + usando encode()
        # Se enviarán el código de operación, nombre de usuario, alias y fecha de nacimiento
        cop = b'REGISTER\0'
        usn = f"{client._username}\0".encode()
        usa = f"{user}\0".encode()
        usbd = f"{client._date}\0".encode()
        
        # Se procede a enviar los datos individualmente por el socket
        try:
            sock.sendall(cop)
            sock.sendall(usn)
            sock.sendall(usa)
            sock.sendall(usbd)
        except Exception as e:
            print("Error al enviar datos en el socket: ", e)

        # Ahora recibimos la respuesta del servidor usando la función vista en los ejemplos, que permite
        # leer números directamente del socket
        try:
            res = int(client.recvMessage(sock), 10)
        except Exception as e:
            res = 2
            print("Error al leer datos del socket: ", e)

        # Dependiendo de la respuesta dada, el valor de retorno será distinto
        #CASO 0: EXITO
        if res == 0:
            window['_SERVER_'].print("s> REGISTER OK")
            sock.close()
            return client.RC.OK
        #CASO 1: REGISTRADO PREVIAMENTE
        if res == 1:
            window['_SERVER_'].print("s> USERNAME IN USE")
            client.clearRegisterData()
            sock.close()
            return client.RC.USER_ERROR
        #OTROS CASOS (INCLUYE TANTO RES = 2 COMO OTROS VALORES INESPERADOS)
        else:
            window['_SERVER_'].print("s> REGISTER FAIL")
            client.clearRegisterData()
            sock.close()
            return client.RC.ERROR

        # --FIN CONEXION--

    # *
    # 	 * @param user - User name to unregister from the system
    # 	 *
    # 	 * @return OK if successful
    # 	 * @return USER_ERROR if the user does not exist
    # 	 * @return ERROR if another error occurred
    @staticmethod
    def  unregister(user, window):
        # Creamos el socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port) 
        # --INICIO CONEXION--
        try:
            sock.connect(server_address)
        except Exception as e:
            print("Error de conexión con el servidor: ", e)

        # Se enviarán el código de operación y el alias
        cop = b'UNREGISTER\0'
        usa = str(user + '\0').encode()

        try:
            sock.sendall(cop)
            sock.sendall(usa)
        except Exception as e:
            print("Error al enviar datos en el socket: ", e)

        try:
            res = int(client.recvMessage(sock), 10)
        except Exception as e:
            res = 2
            print("Error al leer datos del socket: ", e)

        # Dependiendo de la respuesta dada, el valor de retorno será distinto
        #CASO 0: EXITO

        if res == 0:
            window['_SERVER_'].print("s> UNREGISTER OK")
            client.clearRegisterData() 
            sock.close()
            return client.RC.OK
        #CASO 1: REGISTRADO PREVIAMENTE
        if res == 1:
            window['_SERVER_'].print("s> USER DOES NOT EXIST")
            sock.close()
            return client.RC.USER_ERROR
        #OTROS CASOS (INCLUYE TANTO RES = 2 COMO OTROS VALORES INESPERADOS)
        else:
            window['_SERVER_'].print("s> UNREGISTER FAIL")
            sock.close()
            return client.RC.ERROR

        # --FIN CONEXION--


    # *
    # * @param user - User name to connect to the system
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user does not exist or if it is already connected
    # * @return ERROR if another error occurred
    @staticmethod
    def  connect(user, window):
        # Creamos el socket para escuchar los mensajes mientras estamos connectados 
        listenSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        listenSocket_address = ("localhost", 0) 
        listenSocket.bind(listenSocket_address) # Con el puerto 0, se asigna un puerto libre aleatorio
        thread = threading.Thread(target=client.messageListen, args=(listenSocket,))
        thread.start()

        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port)
        try:
            sock.connect(server_address)
        except Exception as e:
            print("Error de conexión con el servidor: ", e)
        
        # Se enviarán el código de operación y el alias
        cop = b'CONNECT\0'
        usa = f"{user}\0".encode()
        port = f"{listenSocket.getsockname()[1]}\0".encode() 

        try:
            sock.sendall(cop)
            sock.sendall(usa)
            sock.sendall(port)
        except Exception as e:
            print("Error al enviar datos en el socket: ", e)

        try:
            res = int(client.recvMessage(sock), 10)
        except Exception as e:
            res = 3
            print("Error al leer datos del socket: ", e)

        if res == 0: 
            window['_SERVER_'].print("s> CONNECT OK")
            sock.close()
            return client.RC.OK
        elif res == 1:
            window['_SERVER_'].print("s> CONNECT FAIL, USER DOES NOT EXIST")
            sock.close()
            return client.RC.USER_ERROR
        elif res == 2:
            window['_SERVER_'].print("s> USER ALREADY CONNECTED")
            sock.close()
            return client.RC.ERROR
        else:
            window['_SERVER_'].print("s> CONNECT FAIL")
            sock.close()
            return 3 #He puesto 3 porque aqui hay que contemplar 4 casos de error

    # *
    # * @param user - User name to disconnect from the system
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user does not exist
    # * @return ERROR if another error occurred
    @staticmethod
    def  disconnect(user, window):
   # Creamos el socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port) 
        # --INICIO CONEXION--
        try:
            sock.connect(server_address)
        except Exception as e:
            print("Error de conexión con el servidor: ", e)

        # Se enviarán el código de operación y el alias
        cop = b'DISCONNECT\0'
        usa = f"{user}\0".encode()

        try:
            sock.sendall(cop)
            sock.sendall(usa)
        except Exception as e:
            print("Error al enviar datos en el socket: ", e)

        try:
            res = int(client.recvMessage(sock), 10)
        except Exception as e:
            res = 3
            print("Error al leer datos del socket: ", e)

        # Dependiendo de la respuesta dada, el valor de retorno será distinto
        #CASO 0: EXITO
        if res == 0: 
            window['_SERVER_'].print("s> DISCONNECT OK")
            sock.close()
            return client.RC.OK
        elif res == 1:
            window['_SERVER_'].print("s> DISCONNECT FAIL / USER DOES NOT EXIST")
            sock.close()
            return client.RC.USER_ERROR
        elif res == 2:
            window['_SERVER_'].print("s> DISCONNECT FAIL / USER NOT CONNECT")
            sock.close()
            return client.RC.ERROR
        else:
            window['_SERVER_'].print("s> DISCONNECT FAIL")
            sock.close()
            return 3 

    # *
    # * @param user    - Receiver user name
    # * @param message - Message to be sent
    # *
    # * @return OK if the server had successfully delivered the message
    # * @return USER_ERROR if the user is not connected (the message is queued for delivery)
    # * @return ERROR the user does not exist or another error occurred
    @staticmethod
    def  send(user, message, window):
   # Creamos el socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port) 
        # --INICIO CONEXION--
        try:
            sock.connect(server_address)
        except Exception as e:
            print("Error de conexión con el servidor: ", e)

        # Se enviarán el código de operación y el alias
        cop = b'SEND\0'
        env = f"{client._alias}\0".encode()
        recv = f"{user}\0".encode()
        messg = f"{message}\0".encode()

        try:
            sock.sendall(cop)
            sock.sendall(env)
            sock.sendall(recv)
            sock.sendall(messg)

        except Exception as e:
            print("Error al enviar datos en el socket: ", e)

        try:
            res = int(client.recvMessage(sock), 10)
        except Exception as e:
            res = 2
            print("Error al leer respuesta del socket: ", e)

        # Dependiendo de la respuesta dada, el valor de retorno será distinto
        #CASO 0: EXITO
        if res == 0: 
            try:
                mess_id = int(client.recvFullMessage(sock), 10)
            except Exception as e:
                window['_SERVER_'].print("s> SEND FAIL")
                sock.close()
                return client.RC.ERROR
            
            window['_SERVER_'].print("s> SEND OK - MESSAGE " + str(mess_id))
            sock.close()
            return client.RC.OK
        elif res == 1:
            window['_SERVER_'].print("s> SEND FAIL / USER DOES NOT EXIST")
            sock.close()
            return client.RC.USER_ERROR
        else:
            window['_SERVER_'].print("s> SEND FAIL")
            sock.close()
            return client.RC.ERROR

    # *
    # * @param user    - Receiver user name
    # * @param message - Message to be sent
    # * @param file    - file  to be sent

    # *
    # * @return OK if the server had successfully delivered the message
    # * @return USER_ERROR if the user is not connected (the message is queued for delivery)
    # * @return ERROR the user does not exist or another error occurred
    @staticmethod
    def  sendAttach(user, message, file, window):
        window['_SERVER_'].print("s> SENDATTACH MESSAGE OK")
        print("SEND ATTACH " + user + " " + message + " " + file)
        #  Write your code here
        return client.RC.ERROR

    @staticmethod
    def  connectedUsers(window):
        connectedUsers = []
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port) 
        # --INICIO CONEXION--
        try:
            sock.connect(server_address)
        except Exception as e:
            print("Error de conexión con el servidor: ", e)

        # Se enviarán el código de operación y el alias
        cop = b'CONNECTEDUSERS\0'
        usa = f"{client._alias}\0".encode()

        try:
            sock.sendall(cop)
            sock.sendall(usa)
        except Exception as e:
            print("Error al enviar datos en el socket: ", e)

        try:
            res = int(client.recvMessage(sock), 10)
        except Exception as e:
            res = 3
            usersCount = 0 
            print("Error al leer datos del socket: ", e)


        


        # Dependiendo de la respuesta dada, el valor de retorno será distinto
        #CASO 0: EXITO
        if res == 0: 
            try:
                usersCount = int(client.recvMessage(sock), 10)
            except Exception as e:
                usersCount = 0 
                print("Error al leer datos del socket: ", e)

            while usersCount > 0: 
                connectedUsers.append(client.recvMessage(sock))
                usersCount -= 1
            
            window['_SERVER_'].print(f's> CONNECTED USERS ({len(connectedUsers)}) OK - {", ".join(connectedUsers)}')
            sock.close()
            return client.RC.OK
        elif res == 1:
            window['_SERVER_'].print("s> CONNECTED USERS FAIL / USER IS NOT CONNECTED")
            sock.close()
            return client.RC.USER_ERROR
        else:
            window['_SERVER_'].print("s> CONNECTED USERS FAIL")
            sock.close()
            return client.RC.ERROR



    @staticmethod
    def window_register():
        layout_register = [[sg.Text('Ful Name:'),sg.Input('Text',key='_REGISTERNAME_', do_not_clear=True, expand_x=True)],
                            [sg.Text('Alias:'),sg.Input('Text',key='_REGISTERALIAS_', do_not_clear=True, expand_x=True)],
                            [sg.Text('Date of birth:'),sg.Input('',key='_REGISTERDATE_', do_not_clear=True, expand_x=True, disabled=True, use_readonly_for_disable=False),
                            sg.CalendarButton("Select Date",close_when_date_chosen=True, target="_REGISTERDATE_", format='%d-%m-%Y',size=(10,1))],
                            [sg.Button('SUBMIT', button_color=('white', 'blue'))]
                            ]

        layout = [[sg.Column(layout_register, element_justification='center', expand_x=True, expand_y=True)]]

        window = sg.Window("REGISTER USER", layout, modal=True)
        choice = None

        while True:
            event, values = window.read()

            if (event in (sg.WINDOW_CLOSED, "-ESCAPE-")):
                break

            if event == "SUBMIT":
                if(values['_REGISTERNAME_'] == 'Text' or values['_REGISTERNAME_'] == '' or values['_REGISTERALIAS_'] == 'Text' or values['_REGISTERALIAS_'] == '' or values['_REGISTERDATE_'] == ''):
                    sg.Popup('Registration error', title='Please fill in the fields to register.', button_type=5, auto_close=True, auto_close_duration=1)
                    continue

                client._username = values['_REGISTERNAME_']
                client._alias = values['_REGISTERALIAS_']
                client._date = values['_REGISTERDATE_']
                break
        window.Close()


    # *
    # * @brief Prints program usage
    @staticmethod
    def usage() :
        print("Usage: python3 py -s <server> -p <port>")


    # *
    # * @brief Parses program execution arguments
    @staticmethod
    def  parseArguments(argv) :
        parser = argparse.ArgumentParser()
        parser.add_argument('-s', type=str, required=True, help='Server IP')
        parser.add_argument('-p', type=int, required=True, help='Server Port')
        args = parser.parse_args()

        if (args.s is None):
            parser.error("Usage: python3 py -s <server> -p <port>")
            return False

        if ((args.p < 1024) or (args.p > 65535)):
            parser.error("Error: Port must be in the range 1024 <= port <= 65535");
            return False;

        client._server = args.s
        client._port = args.p

        return True


    def main(argv):

        if (not client.parseArguments(argv)):
            client.usage()
            exit()

        lay_col = [[sg.Button('REGISTER',expand_x=True, expand_y=True),
                sg.Button('UNREGISTER',expand_x=True, expand_y=True),
                sg.Button('CONNECT',expand_x=True, expand_y=True),
                sg.Button('DISCONNECT',expand_x=True, expand_y=True),
                sg.Button('CONNECTED USERS',expand_x=True, expand_y=True)],
                [sg.Text('Dest:'),sg.Input('User',key='_INDEST_', do_not_clear=True, expand_x=True),
                sg.Text('Message:'),sg.Input('Text',key='_IN_', do_not_clear=True, expand_x=True),
                sg.Button('SEND',expand_x=True, expand_y=False)],
                [sg.Text('Attached File:'), sg.In(key='_FILE_', do_not_clear=True, expand_x=True), sg.FileBrowse(),
                sg.Button('SENDATTACH',expand_x=True, expand_y=False)],
                [sg.Multiline(key='_CLIENT_', disabled=True, autoscroll=True, size=(60,15), expand_x=True, expand_y=True),
                sg.Multiline(key='_SERVER_', disabled=True, autoscroll=True, size=(60,15), expand_x=True, expand_y=True)],
                [sg.Button('QUIT', button_color=('white', 'red'))]
            ]


        layout = [[sg.Column(lay_col, element_justification='center', expand_x=True, expand_y=True)]]

        window = sg.Window('Messenger', layout, resizable=True, finalize=True, size=(1000,400))
        window.bind("<Escape>", "-ESCAPE-")


        while True:
            event, values = window.Read()

            if (event in (None, 'QUIT')) or (event in (sg.WINDOW_CLOSED, "-ESCAPE-")):
                sg.Popup('Closing Client APP', title='Closing', button_type=5, auto_close=True, auto_close_duration=1)
                break

            #if (values['_IN_'] == '') and (event != 'REGISTER' and event != 'CONNECTED USERS'):
             #   window['_CLIENT_'].print("c> No text inserted")
             #   continue

            if (client._alias == None or client._username == None or client._alias == 'Text' or client._username == 'Text' or client._date == None) and (event != 'REGISTER'):
                sg.Popup('NOT REGISTERED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
                continue

            if (event == 'REGISTER'):
                client.window_register()

                if (client._alias == None or client._username == None or client._alias == 'Text' or client._username == 'Text' or client._date == None):
                    sg.Popup('NOT REGISTERED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
                    continue

                window['_CLIENT_'].print('c> REGISTER ' + client._alias)
                client.register(client._alias, window)

            elif (event == 'UNREGISTER'):
                window['_CLIENT_'].print('c> UNREGISTER ' + client._alias)
                client.unregister(client._alias, window)


            elif (event == 'CONNECT'):
                window['_CLIENT_'].print('c> CONNECT ' + client._alias)
                client.connect(client._alias, window)


            elif (event == 'DISCONNECT'):
                window['_CLIENT_'].print('c> DISCONNECT ' + client._alias)
                client.disconnect(client._alias, window)


            elif (event == 'SEND'):
                window['_CLIENT_'].print('c> SEND ' + values['_INDEST_'] + " " + values['_IN_'])

                if (values['_INDEST_'] != '' and values['_IN_'] != '' and values['_INDEST_'] != 'User' and values['_IN_'] != 'Text') :
                    client.send(values['_INDEST_'], values['_IN_'], window)
                else :
                    window['_CLIENT_'].print("Syntax error. Insert <destUser> <message>")


            elif (event == 'SENDATTACH'):

                window['_CLIENT_'].print('c> SENDATTACH ' + values['_INDEST_'] + " " + values['_IN_'] + " " + values['_FILE_'])

                if (values['_INDEST_'] != '' and values['_IN_'] != '' and values['_FILE_'] != '') :
                    client.sendAttach(values['_INDEST_'], values['_IN_'], values['_FILE_'], window)
                else :
                    window['_CLIENT_'].print("Syntax error. Insert <destUser> <message> <attachedFile>")


            elif (event == 'CONNECTED USERS'):
                window['_CLIENT_'].print("c> CONNECTEDUSERS")
                client.connectedUsers(window)



            window.Refresh()

        window.Close()


if __name__ == '__main__':
    client.main([])
    print("+++ FINISHED +++")
