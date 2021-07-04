# Using any device in table, set the same GPS coord only let parameter "pwr=0" 
wget -t 1 http://www.trackingde.tech/postdata.php/postdata?"uid=DE000022&fwv=1.0.0&coord=0.000001, 0.000001&temp=42&pwr=0"
cat post*
rm post*
