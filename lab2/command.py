from enum import Enum
from typing import List
from constants import *
import os, base64


class CommandType(Enum):
    GET_LIST = "get_file_listing"
    GET_METADATA = "get_metadata"
    GET_SLICE = "get_slice"
    QUIT = "quit"
    SUMA = "suma"


class CommandResult(object):
    status: StatusCode
    result_message: str

    def __init__(self, status: StatusCode, result_message: str = "") -> None:
        self.status = status
        self.result_message = result_message

    def __str__(self) -> str:
        if self.result_message != "":
            return f"{self.status.value[0]} {self.status.value[1]}{EOL}{self.result_message}{EOL}"
        return f"{self.status.value[0]} {self.status.value[1]}{EOL}"


class CommandExecutor(object):
    command: CommandType
    args: List[str]
    serve_directory: str

    def __init__(
        self, command: CommandType, args: List[str], serve_directory: str
    ) -> None:
        self.command = command
        self.args = args
        self.serve_directory = serve_directory

    def execute(self) -> CommandResult:
        if self.command == CommandType.GET_LIST:
            return self.handle_get_list()
        if self.command == CommandType.GET_METADATA:
            return self.handle_get_metadata()
        if self.command == CommandType.GET_SLICE:
            return self.handle_get_slice()
        if self.command == CommandType.SUMA:
            return self.handle_get_suma()
        if self.command == CommandType.QUIT:
            return self.handle_quit()
        return CommandResult(StatusCode.INTERNAL_ERROR)

    def handle_get_suma(self) -> CommandResult:
        if len(self.args) != 2:
            return CommandResult(StatusCode.INVALID_ARGUMENTS)
        
        try:
            suma = int(self.args[0]) + int(self.args[1])
        except:
            return CommandResult(StatusCode.INVALID_ARGUMENTS)
            
        return CommandResult(StatusCode.CODE_OK, f"{suma}")

    def handle_get_list(self) -> CommandResult:
        files = os.listdir(self.serve_directory)
        filenames = ""
        for filename in files:
            filenames += filename + EOL
        return CommandResult(StatusCode.CODE_OK, filenames)

    def handle_get_metadata(self) -> CommandResult:
        if len(self.args) != 1:
            return CommandResult(StatusCode.INVALID_ARGUMENTS)

        filename = self.serve_directory + self.args[0]
        if not os.path.exists(filename):
            return CommandResult(StatusCode.FILE_NOT_FOUND)

        size = os.path.getsize(filename)
        return CommandResult(StatusCode.CODE_OK, f"{size}")

    def handle_get_slice(self) -> CommandResult:
        if len(self.args) != 3:
            return CommandResult(StatusCode.INVALID_ARGUMENTS)

        filename = self.serve_directory + self.args[0]
        try:
            offset = int(self.args[1])
            size = int(self.args[2])
        except:
            return CommandResult(StatusCode.INVALID_ARGUMENTS)

        if not os.path.exists(filename):
            return CommandResult(StatusCode.FILE_NOT_FOUND)

        try:
            with open(filename, "rb") as file:
                file_size = os.path.getsize(filename)
                if offset >= file_size:
                    return CommandResult(StatusCode.BAD_OFFSET)

                file.seek(offset)
                data = file.read(size)
                b64_data = base64.b64encode(data)
                return CommandResult(StatusCode.CODE_OK, b64_data.decode("ascii"))
        except Exception:
            return CommandResult(StatusCode.INTERNAL_ERROR)

    def handle_quit(self) -> CommandResult:
        if len(self.args) != 0:
            return CommandResult(StatusCode.INVALID_ARGUMENTS)
        return CommandResult(StatusCode.CODE_OK)
