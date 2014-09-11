#bid
curl -d @bid_request1.json http://localhost:18142/auctions --header "x-openrtb-version:2.1" -vv -H "Content-Type:application/json"
curl -d @bid_request2.json http://localhost:18142/auctions --header "x-openrtb-version:2.1" -vv -H "Content-Type:application/json"
curl -d @bid_request3.json http://localhost:18142/auctions --header "x-openrtb-version:2.1" -vv -H "Content-Type:application/json"

#win
curl -d @win.json http://localhost:18143 -vv -H "Content-Type:application/json"

#campaign
curl -d @campaign.json http://localhost:18144 -vv -H "Content-Type:application/json"

