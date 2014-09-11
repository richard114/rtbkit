#!/bin/bash

for i in {1..200}
do
    curl -d @bid_request3.json http://localhost:18142/auctions --header "x-openrtb-version:2.1" -vv -H "Content-Type:application/json" 2>&1 | grep -e "HTTP/1.1 "
done
