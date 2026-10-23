#!/usr/bin/env python
# encoding: utf-8
# Revisión 2019 (a Python 3 y base64): Pablo Ventura
# Revisión 2014 Carlos Bederián
# Revisión 2011 Nicolás Wolovick
# Copyright 2008-2010 Natalia Bidart y Daniel Moisset
# $Id: server.py 656 2013-03-18 23:49:11Z bc $

import optparse
import socket
import sys
from connection import Connection
import threading
from constants import *


class Server(object):
    """
    El servidor, que crea y atiende el socket en la dirección y puerto
    especificados donde se reciben nuevas conexiones de clientes.
    """

    socket_instance = None
    directory: str

    def __init__(self, addr=DEFAULT_ADDR, port=DEFAULT_PORT,
                 directory=DEFAULT_DIR):
        print("Serving %s on %s:%s." % (directory, addr, port))
        
        if(directory.endswith("/")): 
            self.directory = directory
        else: 
            self.directory = directory + "/"
        
        try:
            self.socket_instance = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket_instance.bind((addr, port))
            self.socket_instance.listen(10)
        except Exception as e:
            print("Error iniciando el servidor, {}".format(e))
            self.socket_instance = None

    def serve(self):
        """
        Loop principal del servidor. Se acepta una conexión a la vez
        y se espera a que concluya antes de seguir.
        """

        if(self.socket_instance is None):
            print("No se puede servir requests si el servidor no se pudo iniciar.")
            sys.exit(1)

        while True:
            # Nos aseguramos de la integridad del servidor envolviendo el loop principal en un try catch.
            try:
                client = self.socket_instance.accept()
                print("Conexion de un cliente recibida.")
                connection = Connection(client[0], self.directory)
                connection_thread = threading.Thread(target=connection.handle)
                connection_thread.start()
                # sin punto estrella
                # connection.handle()
                print("Conexion del cliente enviada al Worker.")
            except Exception as e:
                print("Error iniciando la conexion con un cliente, {}".format(e))

            


def main():
    """Parsea los argumentos y lanza el server"""

    parser = optparse.OptionParser()
    parser.add_option(
        "-p", "--port",
        help="Número de puerto TCP donde escuchar", default=DEFAULT_PORT)
    parser.add_option(
        "-a", "--address",
        help="Dirección donde escuchar", default=DEFAULT_ADDR)
    parser.add_option(
        "-d", "--datadir",
        help="Directorio compartido", default=DEFAULT_DIR)

    options, args = parser.parse_args()
    if len(args) > 0:
        parser.print_help()
        sys.exit(1)
    try:
        port = int(options.port)
    except ValueError:
        sys.stderr.write(
            "Numero de puerto invalido: %s\n" % repr(options.port))
        parser.print_help()
        sys.exit(1)

    server = Server(options.address, port, options.datadir)
    server.serve()


if __name__ == '__main__':
    main()
