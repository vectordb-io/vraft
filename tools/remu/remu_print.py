import gdb

class RemuPrintCommand(gdb.Command):
    """Custom GDB command to call RemuPrint function."""

    def __init__(self):
        super(RemuPrintCommand, self).__init__("remu-print", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        gdb.execute("call RemuPrint()")

class RemuCheckCommand(gdb.Command):
    """Custom GDB command to call RemuCheck function."""

    def __init__(self):
        super(RemuCheckCommand, self).__init__("remu-check", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        gdb.execute("call RemuCheck()")

# Register the custom commands with GDB
RemuPrintCommand()
RemuCheckCommand()

print("Custom GDB commands `remu-print` and `remu-check` registered successfully.")
