import gdb

class MyDumpCommand(gdb.Command):
    def __init__(self):
        super(MyDumpCommand, self).__init__("remu-print", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        gdb.execute("call RemuPrint()")

MyDumpCommand()