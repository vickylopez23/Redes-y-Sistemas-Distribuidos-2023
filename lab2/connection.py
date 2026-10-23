# encoding: utf-8
# Revisión 2019 (a Python 3 y base64): Pablo Ventura
# Copyright 2014 Carlos Bederián
# $Id: connection.py 455 2011-05-01 00:32:09Z carlos $

import socket
from constants import *
from command import *


class Connection(object):
    """
    Conexión punto a punto entre el servidor y un cliente.
    Se encarga de satisfacer los pedidos del cliente hasta
    que termina la conexión.
    """

    connection: socket.socket
    directory: str

    def __init__(self, connection: socket.socket, directory: str):
        self.connection = connection
        self.directory = directory

    def response_back(self, status: StatusCode):
        self.connection.send(
            f"{status.value[0]} {status.value[1]}{EOL}".encode("ascii")
        )

    def handle(self):
        """
        Atiende eventos de la conexión hasta que termina.
        """

        try:
            data = ""
            last_index = 0
            expecting_EOL = False
            commands = []
            while True:
                # recibe un msj
                data += self.connection.recv(4096).decode("ascii")
                while last_index < len(data):
                    char = data[last_index]
                    if not expecting_EOL:
                        if char == "\n":
                            self.response_back(StatusCode.BAD_EOL)
                            data = ""
                            last_index = 0
                            expecting_EOL = False
                            commands = []
                            break
                        elif char == "\r":
                            expecting_EOL = True
                    else:
                        if char == "\r":
                            self.response_back(StatusCode.BAD_EOL)
                            data = ""
                            last_index = 0
                            expecting_EOL = False
                            commands = []
                            break
                        elif char == "\n":
                            commands.append(data[: last_index - 1])
                            data = data[last_index + 1 :]
                            expecting_EOL = False
                            last_index = 0
                            continue
                    last_index += 1

                for command in commands:
                    # separa los comandos por espacios
                    command = command.split(None)
                    if command == []:
                        self.response_back(StatusCode.INVALID_COMMAND)
                        continue
                    try:
                        # Intentamos parsear el Comando en base al string
                        cmd_type = CommandType(command[0])
                    except:
                        self.response_back(StatusCode.INVALID_COMMAND)
                        continue
                    args = command[1:]

                    try:
                        result = CommandExecutor(
                            cmd_type, args, self.directory
                        ).execute()
                    except Exception as e:
                        result = CommandResult(StatusCode.INTERNAL_ERROR)

                    repr(result.__str__())
                    self.connection.send(result.__str__().encode())
                    if (
                        cmd_type == CommandType.QUIT
                        and result.status == StatusCode.CODE_OK
                    ):
                        self.connection.close()
                commands = []
        except Exception:
            return
