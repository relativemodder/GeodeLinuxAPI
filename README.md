# LinuxAPI
Mod which allows you to interact with various Linux Desktops API using dbus on the python side.

# Usage
- Install the mod
- Relogin to your desktop environment
- Profit

# Prerequisites (maybe you already have them installed)
- Python 3.6+
- dbus-python
- glib2
...all of that shit that you propably already have installed on your system.

You need to have a running desktop environment (GNOME, KDE, XFCE, etc.)
You need to have the dbus-daemon running (it should be running by default)

If you're using a window manager that doesn't support XDG autostarting, you might need to start the server manually.


# Manual startup
```bash
python3 ~/linux-api-server.py
```