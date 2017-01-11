#!/bin/bash
sudo apxs -a -i -c mod_waf.c
sudo service apache2 restart
sudo chown -R www-data.www-data /home/dexter/git_working/waf_apache_module/waf
sudo chmod -R 774 /home/dexter/git_working/waf_apache_module/waf
sudo chmod 777 /usr/lib/apache2/modules/mod_waf.so
