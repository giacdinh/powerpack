# Using any device in table, copy GPS coord and change it less than 0.002 
wget -t 1 http://www.trackingde.tech/postdata.php/postdata?"uid=DE000022&fwv=1.0.0&coord=28.217051 , -82.672036&temp=42&pwr=1"
cat post*
rm post*
