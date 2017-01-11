				
				Web Application Firewall
Introduction
---------------------------------------------------------------

A Web Application Firewall (WAF) is an HTTP application firewall, 
which sits in front of the application server and monitors all the 
incoming traffic. The WAF has been implemented as an Apache Module.


Execution steps
---------------------------------------------------------------

-Load the module in the Apache server using the command
$ make all reload

-Change directory path to point to the module's folder in the script build.sh

-Set the module's handler in etc/apache2/apache2.conf
-"waf" is the name of the module.
<Location /waf>
SetHandler waf
</Location>

-Run the following command to build the module.
$./build.sh

-To test the module on the local machine, open browser and type:
localhost/<optional parameters>

Design
---------------------------------------------------------------
The WAF is implemented as an Apache module for Apache HTTP server.
The WAF filters out malicious requests in two phases, signature
checks for known attacks and anomaly detection for unknown attacks.

Signature:
The known attack formats are defined in the signatures file. The WAF
checks the request for malicious attacks based on the signatures and 
guards against them. New signatures can be added in the file.

Anomaly Detection:
For never seen before attacks, we first collect information about 
legitimate traffic and then later based on the collected information,
requests which look vastly different from the legitimate requests 
are discarded.

Implementation
---------------------------------------------------------------
The signatures are defined in a file in an encoded format which are 
used by the WAF to check for attacks.
For anomaly detection, the collected information about legitimate traffic
stored in file system. The data stored in files is encrypted so as to 
avoid misuse of the data from the adversaries.

Contributors
---------------------------------------------------------------
Amogh Avadhani
Ankit Agrahari
Prachi Poddar
Sudeshna Pal


-------------------Copyright 2016, All rights reserved----------------------------
