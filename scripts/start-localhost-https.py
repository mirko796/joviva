#!/usr/bin/env python3
# python3 update of https://gist.github.com/dergachev/7028596
# Create a basic certificate using openssl: 
#     openssl req -new -x509 -keyout server.pem -out server.pem -days 365 -nodes
# Or to set CN, SAN and/or create a cert signed by your own root CA: https://thegreycorner.com/pentesting_stuff/writeups/selfsignedcert.html

import http.server
import ssl
import os

scriptdir = os.path.dirname(os.path.realpath(__file__))
certpath = scriptdir+'/dummy-cert.pem'
httpd = http.server.HTTPServer(('0.0.0.0', 443), http.server.SimpleHTTPRequestHandler)
httpd.socket = ssl.wrap_socket (httpd.socket, certfile=certpath, server_side=True)
print('Starting server on https://0.0.0.0:443')
print('Cert file: '+certpath)
print('Press Ctrl+C to stop')
httpd.serve_forever()