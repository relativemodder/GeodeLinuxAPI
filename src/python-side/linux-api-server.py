#!/usr/bin/env python3

import os

try:
    import json
    import random
    import socket
    import subprocess

    import dbus
    import dbus.mainloop.glib

    from gi.repository import GLib

    from http.server import HTTPServer, BaseHTTPRequestHandler

except ImportError as e:
    with open('/tmp/linux-api-server-error-report.txt', 'w') as f:
        f.write(str(e))
    os._exit(-1)


loop = GLib.MainLoop()
dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

bus = dbus.SessionBus()

dbus_response = None


def handle_dbus_response(response, results, object_path):
    global dbus_response
    dbus_response = results
    loop.quit()


def remove_file_prefix(original: str) -> str:
    return original.replace('file://', '')


class LinuxAPIServer(BaseHTTPRequestHandler):
    def bus_call_picker(
            self,
            open_ = False, 
            save_ = False,
            multiple_ = False,
            directory_ = False,
            proposed_name_ = ""
        ):
        global bus
        global loop 
        global dbus_response

        obj = bus.get_object('org.freedesktop.portal.Desktop', '/org/freedesktop/portal/desktop')
        inter = dbus.Interface(obj, 'org.freedesktop.portal.FileChooser')

        if open_:
            inter.OpenFile(
                '', 
                'Open File' if not multiple_ else 'Select files', 
                { 
                    'multiple': multiple_,
                    'directory': directory_
                }
            )
        elif save_:
            inter.SaveFile('', 'Save File', {
                'current_name': proposed_name_
            })
        else:
            raise ValueError("Invalid action")

        bus.add_signal_receiver(
            handle_dbus_response,
            signal_name='Response',
            dbus_interface='org.freedesktop.portal.Request',
            bus_name='org.freedesktop.portal.Desktop',
            path_keyword='object_path'
        )

        loop.run()

        return dbus_response
    

    def bus_open_file_manager(self, location: str):
        global bus

        obj = bus.get_object('org.freedesktop.FileManager1', '/org/freedesktop/FileManager1')
        inter = dbus.Interface(obj, 'org.freedesktop.FileManager1')

        return inter.ShowFolders(['file://' + location], '')


    def bus_add_notification(self, title: str, body: str):
        global bus
        global dbus_response

        obj = bus.get_object('org.freedesktop.Notifications', '/org/freedesktop/Notifications')
        inter = dbus.Interface(obj, 'org.freedesktop.Notifications')

        inter.Notify(
            'Geometry Dash', 
            random.randint(1111,9999), 
            'notification-symbolic', 
            title, body, 
            [], {}, 10000
        )


    def _set_headers(self):
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()


    def do_GET(self):
        self._set_headers()
            
        return self.wfile.write(
            json.dumps({
                'status': 'ok'
            }).encode()
        )
    

    def do_POST(self):
        self._set_headers()
        content_length = int(self.headers.get('Content-Length', 0))

        try:
            if content_length == 0:
                raise Exception('No data')

            body = self.rfile.read(content_length)
            data = json.loads(body.decode())

            methods = {
                '/files/save': self.files_save,
                '/files/open': self.files_open,
                '/files/filemanager': self.files_filemanager,
                '/notifications/add': self.notifications_add,
                '/xdg/open': self.xdg_open
            }

            method = methods.get(self.path)
            
            if method is None:
                raise Exception('Unknown path')
            
            return self.wfile.write(
                json.dumps(method(data)).encode()
            )
        
        except Exception as e:
            return self.wfile.write(
                json.dumps({
                    'error': str(e)
                }).encode()
            )
    

    def xdg_open(self, params: dict):
        path = params.get('path', None)

        if path is None:
            raise Exception('Missing path')

        result = subprocess.run(['xdg-open', path], stderr=subprocess.PIPE)

        if result.returncode != 0:
            raise Exception(result.stderr.decode())

        return {'status': 'ok'}
    

    def notifications_add(self, params: dict):
        title = params.get('title', None)
        body = params.get('body', None)

        if title is None or body is None:
            raise Exception('Fields title and body are required')

        self.bus_add_notification(title, body)

        return {'status': 'ok'}

   
    def files_filemanager(self, params: dict):
        location = params.get('location', None)

        if location is None:
            raise Exception('Location is required')

        self.bus_open_file_manager(location)
        return {'status': 'ok'}
    

    def files_open(self, params: dict) -> dict:
        multiple_files: bool = params.get('multiple_files', False)
        open_directory: bool = params.get('open_directory', False)

        dbus_result = self.bus_call_picker(
            open_=True,
            directory_=open_directory,
            multiple_=multiple_files
        )

        uris = list(
            map(
                remove_file_prefix, 
                map(str, list(dbus_result['uris']))
            )
        )

        if len(uris) == 0:
            raise Exception('No files selected')
        
        return {'files': uris}


    def files_save(self, params: dict) -> dict:
        file_name: str = params.get('file_name', '')

        dbus_result = self.bus_call_picker(
            save_=True,
            proposed_name_=file_name
        )

        uris = list(
            map(
                remove_file_prefix, 
                map(str, list(dbus_result['uris']))
            )
        )

        if len(uris) == 0:
            raise Exception('No files selected')

        return {'file': uris[0]}
        

def already_running(port: int) -> bool:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    result = sock.connect_ex(('127.0.0.1', port))

    running = result == 0
    sock.close()
    
    return running


def generate_server_port() -> int:
    port_path = '/tmp/linux-api-server-port'

    if os.path.exists(port_path):
        with open(port_path) as fp:
            port_str = fp.read()

        if (already_running(int(port_str))):
            return int(port_str)
    
    port = random.randint(8912, 14888)
    
    with open(port_path, 'w') as fp:
        fp.write(str(port))
        fp.close()

    return port


def run():
    port = generate_server_port()

    if already_running(port):
        print('Server is already running.')
        exit(-1)

    server_address = ('127.0.0.1', port)
    httpd = HTTPServer(server_address, LinuxAPIServer)

    print(f'Starting httpd server on http://{server_address[0]}:{server_address[1]}')
    httpd.serve_forever()


run()