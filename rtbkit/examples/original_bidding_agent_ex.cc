/** bidding_agent_ex.cc                                 -*- C++ -*-
    Rémi Attab, 22 Feb 2013
    Copyright (c) 2013 Datacratic.  All rights reserved.

    Example of a simple fixed-price bidding agent.

*/

#include "rtbkit/common/bids.h"
#include "rtbkit/core/banker/slave_banker.h"
#include "rtbkit/core/agent_configuration/agent_config.h"
#include "rtbkit/plugins/bidding_agent/bidding_agent.h"
#include "soa/service/service_utils.h"

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <iostream>
#include <thread>
#include <chrono>

using namespace std;
using namespace ML;

namespace RTBKIT {

/******************************************************************************/
/* FIXED PRICE BIDDING AGENT                                                  */
/******************************************************************************/

/** Simple bidding agent whose sole purpose in life is to bid 0.1$ CPM on every
    bid requests it sees. It also has a very simple pacer which ensures that we
    always have at most 1$ to spend every 10 seconds.

 */
struct FixedPriceBiddingAgent :
        public BiddingAgent
{
    FixedPriceBiddingAgent(
            std::shared_ptr<Datacratic::ServiceProxies> services,
            const string& serviceName) :
        BiddingAgent(services, serviceName),
        accountSetup(false)
    {}


    void init()
    {
        // We only want to specify a subset of the callbacks so turn the
        // annoying safety belt off.
        strictMode(false);

        onBidRequest = bind(
                &FixedPriceBiddingAgent::bid, this, _1, _2, _3, _4, _5, _6);

        // This component is used to speak with the master banker and pace the
        // rate at which we spend our budget.
        budgetController.init(getServices()->config);
        budgetController.start();

        // Update our pacer every 10 seconds. Note that since this interacts
        // with the budgetController which is only synced up with the router
        // every few seconds, the wait period shouldn't be set too low.
        addPeriodic("FixedPriceBiddingAgent::pace", 10.0,
                [&] (uint64_t) { this->pace(); });

        BiddingAgent::init();
    }

    void start()
    {
        BiddingAgent::start();

        // Build our configuration and tell the world about it.
        setConfig();
    }

    void shutdown()
    {
        BiddingAgent::shutdown();

        budgetController.shutdown();
    }


    /** Sets up an agent configuration for our example. */
    void setConfig()
    {
        config = AgentConfig();

        // Accounts are used to control the allocation of spending budgets for
        // an agent. The whole mechanism is fully generic and can be setup in
        // whatever you feel it bests suits you.
        config.account = {"hello", "world"};

        // Specify the properties of the creatives we are trying to show.
        config.creatives.push_back(Creative::sampleLB);
        config.creatives.push_back(Creative::sampleWS);
        config.creatives.push_back(Creative::sampleBB);

        config.providerConfig["nexage"]["seat"] = "123";
        config.providerConfig["mopub"]["seat"] = "123";
        config.providerConfig["openrtb"]["seat"] = "123";

        config.providerConfig["mopub"]["iurl"] = "https://s0.doubleclick.net/preview/CgkIARCwmOue5igSGQCkoPPIskn71R38hYLyDqk6aNBFIxciZAg/viewad/4381664/1-1024x768_ipad_landscape.jpg";

	// Specify the properties of the creatives we are trying to show.
        //Configure the agent for bidding
        int i = 0;
        for (auto & c : config.creatives) {
            c.providerConfig["nexage"]["adomain"][0] = "nike.com";
	    c.providerConfig["nexage"]["nurl"] = "http://bidder1.alphonso.tv:18143/win?price=${AUCTION_PRICE}&auctionId=${AUCTION_ID}&adSpotId=${AUCTION_ID}-1";

	    c.providerConfig["nexage"]["type"] = "3"; /* Javascript */
	    //c.providerConfig["nexage"]["type"] = "2"; /* XML */
            c.providerConfig["nexage"]["cid"] = "nike";

            // Need to remove AUCTION_PRICE
#if 0
            if(i == 1){ /* 300x50 */
                cout << "short iPhone banner 300x50 at index:" << i << endl;

                c.providerConfig["nexage"]["crid"] = "300x50-landrover";
	        c.providerConfig["nexage"]["iurl"] = "http://s0.2mdn.net/viewad/4381664/300x50_banner_phone_portrait.jpg";

                //c.providerConfig["nexage"]["adm"]  = "<SCRIPT language=\"JavaScript1.1\" SRC=\"http://ad.doubleclick.net/adj/N9515.1370339FLIPBOARD.COM/B8151706.109207190;sz=300x50;ord=[timestamp]?price=${AUCTION_PRICE}\"> </SCRIPT> <NOSCRIPT> <A HREF=\"http://ad.doubleclick.net/jump/N9515.1370339FLIPBOARD.COM/B8151706.109207190;sz=300x50;ord=[timestamp]?\"> <IMG SRC=\"http://ad.doubleclick.net/ad/N9515.1370339FLIPBOARD.COM/B8151706.109207190;sz=300x50;ord=[timestamp]?\" BORDER=0 WIDTH=300 HEIGHT=50 ALT=\"Advertisement\"></A> </NOSCRIPT>";

                  c.providerConfig["nexage"]["adm"]  = "<SCRIPT language=\"JavaScript1.1\" SRC=\"http://ad.doubleclick.net/adj/N9515.153321ESPN0/B7965060.105364307;dcapp=1;sz=300x250;ord=12345678?price=${AUCTION_PRICE}\"> </SCRIPT> <NOSCRIPT> <A HREF=\"http://ad.doubleclick.net/jump/N9515.153321ESPN0/B7965060.105364307;dcapp=1;sz=300x250;ord=12345678?\"> <IMG SRC=\"http://ad.doubleclick.net/ad/N9515.153321ESPN0/B7965060.105364307;dcapp=1;sz=300x250;ord=12345678?\" BORDER=0 WIDTH=300 HEIGHT=250 ALT=\"Advertisement\"></A> </NOSCRIPT>";
            } else {
#endif
                cout << "nexage: tall iPhone banner 300x250 at index:" << i << endl;

                c.providerConfig["nexage"]["crid"] = "300x250-nike";
	        c.providerConfig["nexage"]["iurl"] = "http://s0.2mdn.net/viewad/4381664/300x250_landscape.jpg";

                //c.providerConfig["nexage"]["adm"]  = "<SCRIPT language=\"JavaScript1.1\" SRC=\"http://ad.doubleclick.net/adj/N9515.153321ESPN0/B8150894.109207206;dcapp=1;sz=300x250;ord=[timestamp]?price=${AUCTION_PRICE}\"> </SCRIPT> <NOSCRIPT> <A HREF=\"http://ad.doubleclick.net/jump/N9515.153321ESPN0/B8150894.109207206;dcapp=1;sz=300x250;ord=[timestamp]?\"> <IMG SRC=\"http://ad.doubleclick.net/ad/N9515.153321ESPN0/B8150894.109207206;dcapp=1;sz=300x250;ord=[timestamp]?\" BORDER=0 WIDTH=300 HEIGHT=250 ALT=\"Advertisement\"></A> </NOSCRIPT>";


                  //c.providerConfig["nexage"]["adm"]  = "<SCRIPT language=\"JavaScript1.1\" SRC=\"http://ad.doubleclick.net/adj/N9515.153321ESPN0/B7965060.105364307;dcapp=1;sz=300x250;ord=12345678?price=${AUCTION_PRICE}\"> </SCRIPT> <NOSCRIPT> <A HREF=\"http://ad.doubleclick.net/jump/N9515.153321ESPN0/B7965060.105364307;dcapp=1;sz=300x250;ord=12345678?\"> <IMG SRC=\"http://ad.doubleclick.net/ad/N9515.153321ESPN0/B7965060.105364307;dcapp=1;sz=300x250;ord=12345678?\" BORDER=0 WIDTH=300 HEIGHT=250 ALT=\"Advertisement\"></A> </NOSCRIPT>";


                  //c.providerConfig["nexage"]["crid"] = "nike-magista-video";

	          // Doesn't work c.providerConfig["nexage"]["iurl"] = "http://gcdn.2mdn.net/videoplayback/id/f86ed4cad35c5227/itag/18/source/doubleclick_dmm/ratebypass/yes/ip/0.0.0.0/ipbits/0/expire/3546201753/sparams/id,itag,source,ratebypass,ip,ipbits,expire/signature/87680762A0C8622C9BBD837F5010DD773887EABD.5DF1703C28782003CB84D8B97CD5026E9AF0EE83/key/ck2/file/file.mp4";

                   c.providerConfig["nexage"]["adm"]  = "<SCRIPT language=\"JavaScript1.1\" SRC=\"http://ad.doubleclick.net/adj/N9515.153321ESPN0/B7965060.105364307;dcapp=1;sz=300x250;ord=12345678?price=${AUCTION_PRICE}\"> </SCRIPT> <NOSCRIPT> <A HREF=\"http://ad.doubleclick.net/jump/N9515.153321ESPN0/B7965060.105364307;dcapp=1;sz=300x250;ord=12345678?\"> <IMG SRC=\"http://ad.doubleclick.net/pfadx/N8893.1882427ALPHONSO.TV/B8020244.109226087;sz=0x0;ord=12345678;dcmt=text/xml\" BORDER=0 WIDTH=300 HEIGHT=250 ALT=\"Advertisement\"></A> </NOSCRIPT>";

#if 0
                  // Doesn't work c.providerConfig["nexage"]["adm"]  = "%3C%3Fxml+version%3D%221.0%22+encoding%3D%22UTF-8%22%3F%3E%0D%0A%3CVAST+xmlns%3Axsi%3D%22http%3A%2F%2Fwww.w3.org%2F2001%2FXMLSchema-instance%22+xsi%3AnoNamespaceSchemaLocation%3D%22vast.xsd%22+version%3D%222.0%22%3E%0D%0A+%3CAd+id%3D%22282492844%22%3E%0D%0A++%3CInLine%3E%0D%0A+++%3CAdSystem%3EDCM%3C%2FAdSystem%3E%0D%0A+++%3CAdTitle%3EIn-Stream+Video%3C%2FAdTitle%3E%0D%0A+++%3CError%3E%3C%21%5BCDATA%5B%5D%5D%3E%3C%2FError%3E%0D%0A+++%3CImpression%3E%3C%21%5BCDATA%5Bhttp%3A%2F%2Fgoogleads4.g.doubleclick.net%2Fpagead%2Fadview%3Fai%3DBu3lBUWSWU-LXCMf3lgeshoCYDQAAAAAQASAAOABQh5LLoQRY1NLqG2DJrpmN7KSAEIIBCWNhLWdvb2dsZcgBBagDAeAEAqAGOuAGsoqDAg%26sigh%3D7ZyprTsnhoc%26adurl%3D%5D%5D%3E%3C%2FImpression%3E%0D%0A+++%3CCreatives%3E%0D%0A++++%3CCreative+sequence%3D%221%22%3E%0D%0A+++++%3CLinear%3E%0D%0A++++++%3CDuration%3E00%3A00%3A30%3C%2FDuration%3E%0D%0A++++++%3CTrackingEvents%3E%0D%0A+++++++%3CTracking+event%3D%22start%22%3E%3C%21%5BCDATA%5Bhttp%3A%2F%2Fad.doubleclick.net%2Factivity%3Bsrc%3D4244786%3Bmet%3D1%3Bv%3D1%3Bpid%3D109226087%3Baid%3D282492844%3Bko%3D0%3Bcid%3D58370388%3Brid%3D58259484%3Brv%3D1%3Btimestamp%3D1532276120%3Becn1%3D1%3Betm1%3D0%3Beid1%3D11%3B%5D%5D%3E%3C%2FTracking%3E%0D%0A+++++++%3CTracking+event%3D%22firstQuartile%22%3E%3C%21%5BCDATA%5Bhttp%3A%2F%2Fad.doubleclick.net%2Factivity%3Bsrc%3D4244786%3Bmet%3D1%3Bv%3D1%3Bpid%3D109226087%3Baid%3D282492844%3Bko%3D0%3Bcid%3D58370388%3Brid%3D58259484%3Brv%3D1%3Btimestamp%3D1532276120%3Becn1%3D1%3Betm1%3D0%3Beid1%3D960584%3B%5D%5D%3E%3C%2FTracking%3E%0D%0A+++++++%3CTracking+event%3D%22midpoint%22%3E%3C%21%5BCDATA%5Bhttp%3A%2F%2Fad.doubleclick.net%2Factivity%3Bsrc%3D4244786%3Bmet%3D1%3Bv%3D1%3Bpid%3D109226087%3Baid%3D282492844%3Bko%3D0%3Bcid%3D58370388%3Brid%3D58259484%3Brv%3D1%3Btimestamp%3D1532276120%3Becn1%3D1%3Betm1%3D0%3Beid1%3D18%3B%5D%5D%3E%3C%2FTracking%3E%0D%0A+++++++%3CTracking+event%3D%22thirdQuartile%22%3E%3C%21%5BCDATA%5Bhttp%3A%2F%2Fad.doubleclick.net%2Factivity%3Bsrc%3D4244786%3Bmet%3D1%3Bv%3D1%3Bpid%3D109226087%3Baid%3D282492844%3Bko%3D0%3Bcid%3D58370388%3Brid%3D58259484%3Brv%3D1%3Btimestamp%3D1532276120%3Becn1%3D1%3Betm1%3D0%3Beid1%3D960585%3B%5D%5D%3E%3C%2FTracking%3E%0D%0A+++++++%3CTracking+event%3D%22complete%22%3E%3C%21%5BCDATA%5Bhttp%3A%2F%2Fad.doubleclick.net%2Factivity%3Bsrc%3D4244786%3Bmet%3D1%3Bv%3D1%3Bpid%3D109226087%3Baid%3D282492844%3Bko%3D0%3Bcid%3D58370388%3Brid%3D58259484%3Brv%3D1%3Btimestamp%3D1532276120%3Becn1%3D1%3Betm1%3D0%3Beid1%3D13%3B%5D%5D%3E%3C%2FTracking%3E%0D%0A+++++++%3CTracking+event%3D%22mute%22%3E%3C%21%5BCDATA%5Bhttp%3A%2F%2Fad.doubleclick.net%2Factivity%3Bsrc%3D4244786%3Bmet%3D1%3Bv%3D1%3Bpid%3D109226087%3Baid%3D282492844%3Bko%3D0%3Bcid%3D58370388%3Brid%3D58259484%3Brv%3D1%3Btimestamp%3D1532276120%3Becn1%3D1%3Betm1%3D0%3Beid1%3D16%3B%5D%5D%3E%3C%2FTracking%3E%0D%0A+++++++%3CTracking+event%3D%22unmute%22%3E%3C%21%5BCDATA%5Bhttp%3A%2F%2Fad.doubleclick.net%2Factivity%3Bsrc%3D4244786%3Bmet%3D1%3Bv%3D1%3Bpid%3D109226087%3Baid%3D282492844%3Bko%3D0%3Bcid%3D58370388%3Brid%3D58259484%3Brv%3D1%3Btimestamp%3D1532276120%3Becn1%3D1%3Betm1%3D0%3Beid1%3D149645%3B%5D%5D%3E%3C%2FTracking%3E%0D%0A+++++++%3CTracking+event%3D%22pause%22%3E%3C%21%5BCDATA%5Bhttp%3A%2F%2Fad.doubleclick.net%2Factivity%3Bsrc%3D4244786%3Bmet%3D1%3Bv%3D1%3Bpid%3D109226087%3Baid%3D282492844%3Bko%3D0%3Bcid%3D58370388%3Brid%3D58259484%3Brv%3D1%3Btimestamp%3D1532276120%3Becn1%3D1%3Betm1%3D0%3Beid1%3D15%3B%5D%5D%3E%3C%2FTracking%3E%0D%0A+++++++%3CTracking+event%3D%22fullscreen%22%3E%3C%21%5BCDATA%5Bhttp%3A%2F%2Fad.doubleclick.net%2Factivity%3Bsrc%3D4244786%3Bmet%3D1%3Bv%3D1%3Bpid%3D109226087%3Baid%3D282492844%3Bko%3D0%3Bcid%3D58370388%3Brid%3D58259484%3Brv%3D1%3Btimestamp%3D1532276120%3Becn1%3D1%3Betm1%3D0%3Beid1%3D19%3B%5D%5D%3E%3C%2FTracking%3E%0D%0A++++++%3C%2FTrackingEvents%3E%0D%0A++++++%3CVideoClicks%3E%0D%0A+++++++%3CClickThrough%3E%3C%21%5BCDATA%5Bhttp%3A%2F%2Fwww.nike.com%2Fus%2Fen_us%2Fc%2Ffootball%2Friskeverything%3Fcp%3Dus_fb_brm_6112014_su14_wcp3_lgf15_vid_N8893.1882427ALPHONSO.TV%5D%5D%3E%3C%2FClickThrough%3E%0D%0A+++++++%3CClickTracking%3E%3C%21%5BCDATA%5Bhttp%3A%2F%2Fadclick.g.doubleclick.net%2Faclk%3Fsa%3DL%26ai%3DBu3lBUWSWU-LXCMf3lgeshoCYDQAAAAAQASAAOABQh5LLoQRY1NLqG2DJrpmN7KSAEIIBCWNhLWdvb2dsZcgBBagDAeAEAqAGOuAGsoqDAg%26num%3D0%26sig%3DAOD64_1Qc4kprxWX6KXUMaBoP2mKzfjLTA%26client%3D%26adurl%3D%5D%5D%3E%3C%2FClickTracking%3E%0D%0A++++++%3C%2FVideoClicks%3E%0D%0A++++++%3CMediaFiles%3E%0D%0A+++++++%3CMediaFile+delivery%3D%22progressive%22+width%3D%22480%22+height%3D%22360%22+type%3D%22video%2Fx-flv%22+bitrate%3D%22429%22+scalable%3D%22false%22+maintainAspectRatio%3D%22false%22%3E%3C%21%5BCDATA%5Bhttp%3A%2F%2Fgcdn.2mdn.net%2Fvideoplayback%2Fid%2F4259e29959d3f65c%2Fitag%2F34%2Fsource%2Fdoubleclick_dmm%2Fratebypass%2Fyes%2Fip%2F0.0.0.0%2Fipbits%2F0%2Fexpire%2F3546786020%2Fsparams%2Fid%2Citag%2Csource%2Cratebypass%2Cip%2Cipbits%2Cexpire%2Fsignature%2F54A718C8D611E01747DCB9A6ADE5222C22015F31.B8EAF7E31DD428631CD1C6342FE72138E925A892%2Fkey%2Fck2%2Ffile%2Ffile.flv%5D%5D%3E%3C%2FMediaFile%3E%0D%0A+++++++%3CMediaFile+delivery%3D%22progressive%22+width%3D%22176%22+height%3D%22144%22+type%3D%22video%2F3gpp%22+bitrate%3D%2253%22+scalable%3D%22false%22+maintainAspectRatio%3D%22false%22%3E%3C%21%5BCDATA%5Bhttp%3A%2F%2Fgcdn.2mdn.net%2Fvideoplayback%2Fid%2F4259e29959d3f65c%2Fitag%2F17%2Fsource%2Fdoubleclick_dmm%2Fratebypass%2Fyes%2Fip%2F0.0.0.0%2Fipbits%2F0%2Fexpire%2F3546786005%2Fsparams%2Fid%2Citag%2Csource%2Cratebypass%2Cip%2Cipbits%2Cexpire%2Fsignature%2F666DC4E63A01B7A499969B0DA578C3EFA8B81AD5.1258F287439722630E0FB4408AC9229814418DBA%2Fkey%2Fck2%2Ffile%2Ffile.3gpp%5D%5D%3E%3C%2FMediaFile%3E%0D%0A+++++++%3CMediaFile+delivery%3D%22progressive%22+width%3D%22320%22+height%3D%22240%22+type%3D%22video%2F3gpp%22+bitrate%3D%22187%22+scalable%3D%22false%22+maintainAspectRatio%3D%22false%22%3E%3C%21%5BCDATA%5Bhttp%3A%2F%2Fgcdn.2mdn.net%2Fvideoplayback%2Fid%2F4259e29959d3f65c%2Fitag%2F36%2Fsource%2Fdoubleclick_dmm%2Fratebypass%2Fyes%2Fip%2F0.0.0.0%2Fipbits%2F0%2Fexpire%2F3546786012%2Fsparams%2Fid%2Citag%2Csource%2Cratebypass%2Cip%2Cipbits%2Cexpire%2Fsignature%2FB3DE8D2CB2D8487BE34AD71CA3642151370E4BDD.2E737F7ADA4FC1DECF0F1E4E04AC7EE1375C02B7%2Fkey%2Fck2%2Ffile%2Ffile.3gpp%5D%5D%3E%3C%2FMediaFile%3E%0D%0A+++++++%3CMediaFile+delivery%3D%22progressive%22+width%3D%22480%22+height%3D%22360%22+type%3D%22video%2Fmp4%22+bitrate%3D%22450%22+scalable%3D%22false%22+maintainAspectRatio%3D%22false%22%3E%3C%21%5BCDATA%5Bhttp%3A%2F%2Fgcdn.2mdn.net%2Fvideoplayback%2Fid%2F4259e29959d3f65c%2Fitag%2F18%2Fsource%2Fdoubleclick_dmm%2Fratebypass%2Fyes%2Fip%2F0.0.0.0%2Fipbits%2F0%2Fexpire%2F3546786015%2Fsparams%2Fid%2Citag%2Csource%2Cratebypass%2Cip%2Cipbits%2Cexpire%2Fsignature%2FABD8FFB9D6A5255374E318D7AFD8D1C541D69D9B.A902E767326701381482F6E0DB12C949D89E7B50%2Fkey%2Fck2%2Ffile%2Ffile.mp4%5D%5D%3E%3C%2FMediaFile%3E%0D%0A++++++%3C%2FMediaFiles%3E%0D%0A+++++%3C%2FLinear%3E%0D%0A++++%3C%2FCreative%3E%0D%0A+++%3C%2FCreatives%3E%0D%0A+++%3CExtensions%3E%0D%0A++++%3CExtension+type%3D%22dart%22%3E%3CAdServingData%3E%0D%0A+%3CDeliveryData%3E%0D%0A++%3CGeoData%3E%3C%21%5BCDATA%5Bct%3DUS%26st%3DVA%26city%3D16629%26dma%3D13%26zp%3D%26bw%3D4%5D%5D%3E%3C%2FGeoData%3E%0D%0A+%3C%2FDeliveryData%3E%0D%0A%3C%2FAdServingData%3E%0D%0A%3C%2FExtension%3E%0D%0A+++%3C%2FExtensions%3E%0D%0A++%3C%2FInLine%3E%0D%0A+%3C%2FAd%3E%0D%0A%3C%2FVAST%3E";
                   // Doesn't work c.providerConfig["nexage"]["adm"]  = "<?xml version=\"1.0\" encoding=\"UTF-8\"?> <VAST xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"vast.xsd\" version=\"2.0\"> <Ad id=\"282492844\"> <InLine> <AdSystem>DCM</AdSystem> <AdTitle>In-Stream Video</AdTitle> <Error><![CDATA[]]></Error> <Impression><![CDATA[http://googleads4.g.doubleclick.net/pagead/adview?ai=Bu3lBUWSWU-LXCMf3lgeshoCYDQAAAAAQASAAOABQh5LLoQRY1NLqG2DJrpmN7KSAEIIBCWNhLWdvb2dsZcgBBagDAeAEAqAGOuAGsoqDAg&sigh=7ZyprTsnhoc&adurl=]]></Impression> <Creatives> <Creative sequence=\"1\"> <Linear> <Duration>00:00:30</Duration> <TrackingEvents> <Tracking event=\"start\"><![CDATA[http://ad.doubleclick.net/activity;src=4244786;met=1;v=1;pid=109226087;aid=282492844;ko=0;cid=58370388;rid=58259484;rv=1;timestamp=1532276120;ecn1=1;etm1=0;eid1=11;]]></Tracking> <Tracking event=\"firstQuartile\"><![CDATA[http://ad.doubleclick.net/activity;src=4244786;met=1;v=1;pid=109226087;aid=282492844;ko=0;cid=58370388;rid=58259484;rv=1;timestamp=1532276120;ecn1=1;etm1=0;eid1=960584;]]></Tracking> <Tracking event=\"midpoint\"><![CDATA[http://ad.doubleclick.net/activity;src=4244786;met=1;v=1;pid=109226087;aid=282492844;ko=0;cid=58370388;rid=58259484;rv=1;timestamp=1532276120;ecn1=1;etm1=0;eid1=18;]]></Tracking> <Tracking event=\"thirdQuartile\"><![CDATA[http://ad.doubleclick.net/activity;src=4244786;met=1;v=1;pid=109226087;aid=282492844;ko=0;cid=58370388;rid=58259484;rv=1;timestamp=1532276120;ecn1=1;etm1=0;eid1=960585;]]></Tracking> <Tracking event=\"complete\"><![CDATA[http://ad.doubleclick.net/activity;src=4244786;met=1;v=1;pid=109226087;aid=282492844;ko=0;cid=58370388;rid=58259484;rv=1;timestamp=1532276120;ecn1=1;etm1=0;eid1=13;]]></Tracking> <Tracking event=\"mute\"><![CDATA[http://ad.doubleclick.net/activity;src=4244786;met=1;v=1;pid=109226087;aid=282492844;ko=0;cid=58370388;rid=58259484;rv=1;timestamp=1532276120;ecn1=1;etm1=0;eid1=16;]]></Tracking> <Tracking event=\"unmute\"><![CDATA[http://ad.doubleclick.net/activity;src=4244786;met=1;v=1;pid=109226087;aid=282492844;ko=0;cid=58370388;rid=58259484;rv=1;timestamp=1532276120;ecn1=1;etm1=0;eid1=149645;]]></Tracking> <Tracking event=\"pause\"><![CDATA[http://ad.doubleclick.net/activity;src=4244786;met=1;v=1;pid=109226087;aid=282492844;ko=0;cid=58370388;rid=58259484;rv=1;timestamp=1532276120;ecn1=1;etm1=0;eid1=15;]]></Tracking> <Tracking event=\"fullscreen\"><![CDATA[http://ad.doubleclick.net/activity;src=4244786;met=1;v=1;pid=109226087;aid=282492844;ko=0;cid=58370388;rid=58259484;rv=1;timestamp=1532276120;ecn1=1;etm1=0;eid1=19;]]></Tracking> </TrackingEvents> <VideoClicks> <ClickThrough><![CDATA[http://www.nike.com/us/en_us/c/football/riskeverything?cp=us_fb_brm_6112014_su14_wcp3_lgf15_vid_N8893.1882427ALPHONSO.TV]]></ClickThrough> <ClickTracking><![CDATA[http://adclick.g.doubleclick.net/aclk?sa=L&ai=Bu3lBUWSWU-LXCMf3lgeshoCYDQAAAAAQASAAOABQh5LLoQRY1NLqG2DJrpmN7KSAEIIBCWNhLWdvb2dsZcgBBagDAeAEAqAGOuAGsoqDAg&num=0&sig=AOD64_1Qc4kprxWX6KXUMaBoP2mKzfjLTA&client=&adurl=]]></ClickTracking> </VideoClicks> <MediaFiles> <MediaFile delivery=\"progressive\" width=\"480\" height=\"360\" type=\"video/x-flv\" bitrate=\"429\" scalable=\"false\" maintainAspectRatio=\"false\"><![CDATA[http://gcdn.2mdn.net/videoplayback/id/4259e29959d3f65c/itag/34/source/doubleclick_dmm/ratebypass/yes/ip/0.0.0.0/ipbits/0/expire/3546786020/sparams/id,itag,source,ratebypass,ip,ipbits,expire/signature/54A718C8D611E01747DCB9A6ADE5222C22015F31.B8EAF7E31DD428631CD1C6342FE72138E925A892/key/ck2/file/file.flv]]></MediaFile> <MediaFile delivery=\"progressive\" width=\"176\" height=\"144\" type=\"video/3gpp\" bitrate=\"53\" scalable=\"false\" maintainAspectRatio=\"false\"><![CDATA[http://gcdn.2mdn.net/videoplayback/id/4259e29959d3f65c/itag/17/source/doubleclick_dmm/ratebypass/yes/ip/0.0.0.0/ipbits/0/expire/3546786005/sparams/id,itag,source,ratebypass,ip,ipbits,expire/signature/666DC4E63A01B7A499969B0DA578C3EFA8B81AD5.1258F287439722630E0FB4408AC9229814418DBA/key/ck2/file/file.3gpp]]></MediaFile> <MediaFile delivery=\"progressive\" width=\"320\" height=\"240\" type=\"video/3gpp\" bitrate=\"187\" scalable=\"false\" maintainAspectRatio=\"false\"><![CDATA[http://gcdn.2mdn.net/videoplayback/id/4259e29959d3f65c/itag/36/source/doubleclick_dmm/ratebypass/yes/ip/0.0.0.0/ipbits/0/expire/3546786012/sparams/id,itag,source,ratebypass,ip,ipbits,expire/signature/B3DE8D2CB2D8487BE34AD71CA3642151370E4BDD.2E737F7ADA4FC1DECF0F1E4E04AC7EE1375C02B7/key/ck2/file/file.3gpp]]></MediaFile> <MediaFile delivery=\"progressive\" width=\"480\" height=\"360\" type=\"video/mp4\" bitrate=\"450\" scalable=\"false\" maintainAspectRatio=\"false\"><![CDATA[http://gcdn.2mdn.net/videoplayback/id/4259e29959d3f65c/itag/18/source/doubleclick_dmm/ratebypass/yes/ip/0.0.0.0/ipbits/0/expire/3546786015/sparams/id,itag,source,ratebypass,ip,ipbits,expire/signature/ABD8FFB9D6A5255374E318D7AFD8D1C541D69D9B.A902E767326701381482F6E0DB12C949D89E7B50/key/ck2/file/file.mp4]]></MediaFile> </MediaFiles> </Linear> </Creative> </Creatives> <Extensions> <Extension type=\"dart\"><AdServingData> <DeliveryData> <GeoData><![CDATA[ct=US&st=VA&city=16629&dma=13&zp=&bw=4]]></GeoData> </DeliveryData> </AdServingData> </Extension> </Extensions> </InLine> </Ad> </VAST>";
#endif



#if 0
            }
#endif

	    c.providerConfig["mopub"]["attr"] = "2";
	    c.providerConfig["mopub"]["type"] = "2"; /* XHTML banner */
	    c.providerConfig["mopub"]["cat"]  = "IAB14";
            c.providerConfig["mopub"]["adomain"][0] = "GoDaddy";
            c.providerConfig["mopub"]["adm"]  = "<SCRIPT LANGUAGE=\"JavaScript1.1\" SRC=\"http://www.adserver.com/blah?and=more&price=${AUCTION_PRICE:BF}</SCRIPT>";
            c.providerConfig["mopub"]["crid"] = "320x250-landrover";

            c.providerConfig["openrtb"]["adomain"][0] = "landrover.com";
	    c.providerConfig["openrtb"]["nurl"] = "http://bidder1.alphonso.tv:18143/win?price=${AUCTION_PRICE}&auctionId=${AUCTION_ID}&adSpotId=${AUCTION_ID}-1";

            cout << "openrtb: tall iPhone banner 300x250 at index:" << i << endl;
	    c.providerConfig["openrtb"]["type"] = "3"; /* Javascript */
            c.providerConfig["openrtb"]["cid"] = "landrover";
            c.providerConfig["openrtb"]["crid"] = "300x250-landrover";
            c.providerConfig["openrtb"]["iurl"] = "http://s0.2mdn.net/viewad/4381664/300x250_landscape.jpg";
            c.providerConfig["openrtb"]["adm"]  = "<SCRIPT language=\"JavaScript1.1\" SRC=\"http://ad.doubleclick.net/adj/N9515.153321ESPN0/B7965060.105364307;dcapp=1;sz=300x250;ord=12345678?price=${AUCTION_PRICE}\"> </SCRIPT> <NOSCRIPT> <A HREF=\"http://ad.doubleclick.net/jump/N9515.153321ESPN0/B7965060.105364307;dcapp=1;sz=300x250;ord=12345678?\"> <IMG SRC=\"http://ad.doubleclick.net/ad/N9515.153321ESPN0/B7965060.105364307;dcapp=1;sz=300x250;ord=12345678?\" BORDER=0 WIDTH=300 HEIGHT=250 ALT=\"Advertisement\"></A> </NOSCRIPT>";

            cout << "creative: " << c.toJson() << endl;
            i++;
        } 

        // Indicate to the router that we want our bid requests to be augmented
        // with our frequency cap augmentor example.
        {
            AugmentationConfig augConfig;

            // Name of the requested augmentor.
            augConfig.name = "frequency-cap-ex";

            // If the augmentor was unable to augment our bid request then it
            // should be filtered before it makes it to our agent.
            augConfig.required = true;

            // Config parameter sent used by the augmentor to determine which
            // tag to set.
            augConfig.config = Json::Value(42);

            // Instruct to router to filter out all bid requests who have not
            // been tagged by our frequency cap augmentor.
            augConfig.filters.include.push_back("pass-frequency-cap-ex");

            config.addAugmentation(augConfig);
        }

        // Configures the agent to only receive 10% of the bid request traffic
        // that matches its filters.
        config.bidProbability = 1.0;

        // Tell the world about our config. We can change the configuration of
        // an agent at any time by calling this function.
        doConfig(config);
    }


    /** Simple fixed price bidding strategy. Note that the router is in charge
        of making sure we stay within budget and don't go bankrupt.
    */
    void bid(
            double timestamp,
            const Id & id,
            std::shared_ptr<RTBKIT::BidRequest> br,
            Bids bids,
            double timeLeftMs,
            const Json::Value & augmentations)
    {
        for (Bid& bid : bids) {

            // In our example, all our creatives are of the different sizes so
            // there should only ever be one biddable creative. Note that that
            // the router won't ask for bids on imp that don't have any
            // biddable creatives.
            ExcAssertEqual(bid.availableCreatives.size(), 1);
            int availableCreative = bid.availableCreatives.front();

            // We don't really need it here but this is how you can get the
            // AdSpot and Creative object from the indexes.
            (void) br->imp[bid.spotIndex];
            (void) config.creatives[availableCreative];

            //Date start = Date::now();
            //cout << "Bid Request at:" << start << endl;
            //cout << "Bid Request contains device model:" << br->device->model << endl;
            //cout << "Bid Request contains device os:" << br->device->os << endl;
            //cout << "Bid Request contains imp:" << br->imp[bid.spotIndex].format() << endl;
            //cout << "Bid is using creative:" << availableCreative << endl;
            //cout << "Bid is using creative:" << config.creatives[availableCreative].toJson() << endl;

            //const char *model = br->device->model.rawData();
            //const char *os = br->device->os.rawData();

            //if ( ((strcmp(os, "iOS")) == 0) || ((strcmp(model, "iPhone")) == 0) || ((strcmp(model, "iPod Touch")) == 0) || ((strcmp(model, "iPad")) == 0) ){
                //cout << "Placing Bid for iOS device" << endl;
            //} else {
                //cout << "Should Not place Bid" << endl;
            //}

            // Create a 0.0001$ CPM bid with our available creative.
            // Note that by default, the bid price is set to 0 which indicates
            // that we don't wish to bid on the given spot.
            bid.bid(availableCreative, MicroUSD(100));
        }

        // A value that will be passed back to us when we receive the result of
        // our bid.
        Json::Value metadata = 42;

        // Send our bid back to the agent.
        doBid(id, bids, metadata);
    }


    /** Simple pacing scheme which allocates 1$ to spend every period. */
    void pace()
    {
        // We need to register our account once with the banker service.
        if (!accountSetup) {
            accountSetup = true;
            budgetController.addAccountSync(config.account);
        }

        // Make sure we have 1$ to spend for the next period.
        budgetController.topupTransferSync(config.account, USD(1));
    }


    AgentConfig config;

    bool accountSetup;
    SlaveBudgetController budgetController;
};

} // namepsace RTBKIT


/******************************************************************************/
/* MAIN                                                                       */
/******************************************************************************/

int main(int argc, char** argv)
{
    using namespace boost::program_options;

    Datacratic::ServiceProxyArguments args;

    options_description options = args.makeProgramOptions();
    options.add_options() ("help,h", "Print this message");

    variables_map vm;
    store(command_line_parser(argc, argv).options(options).run(), vm);
    notify(vm);

    if (vm.count("help")) {
        cerr << options << endl;
        return 1;
    }

    auto serviceProxies = args.makeServiceProxies();
    RTBKIT::FixedPriceBiddingAgent agent(serviceProxies, "fixed-price-agent-ex");
    agent.init();
    agent.start();

    while (true) this_thread::sleep_for(chrono::seconds(10));

    // Won't ever reach this point but this is how you shutdown an agent.
    agent.shutdown();

    return 0;
}

