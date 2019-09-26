from __future__ import print_function
from __future__ import unicode_literals
from future import standard_library
standard_library.install_aliases()
from builtins import str
import http.client
import argparse
import time
import json

def put_handler(ip, nodeport):
    # Establish HTTP connection
    # print("Connecting to => " + ip + ":" + espport)
    # sess = http.client.HTTPConnection(ip + ":" + espport)
    node = http.client.HTTPConnection(ip + ":" + nodeport)

    headers = {'Content-Type': 'text/plain'}
    adc_data = {'adc_reading': '0'}
    hour = -1
    minute = -1

    while True:
        try:
        # Send ADC data to node server
            adc = '1:1000'
            # adc_data['adc_reading'] = adc
            # adc_json = json.dumps(adc_data)
            # node.request("PUT", "/adc", adc_json, headers)
            node.request("PUT", "/", adc, headers);
            print("sending put request")
            resp = node.getresponse()
            resp.read()

        except KeyboardInterrupt:
            print("Quitting . . .")

    # Close HTTP connection
    # sess.close()

if __name__ == '__main__':
    # Configure argument parser
    parser = argparse.ArgumentParser(description='Run HTTPd Test')
    parser.add_argument('IP'  , metavar='IP'  ,    type=str, help='Server IP')
    # parser.add_argument('espport', metavar='espport',    type=str, help='ESP port')
    parser.add_argument('nodeport', metavar='nodeport',    type=str, help='Node port')
    args = vars(parser.parse_args())

    # Get arguments
    ip   = args['IP']
    # espport = args['espport']
    nodeport = args['nodeport']

    # Call PUT handler
    put_handler (ip, nodeport)
