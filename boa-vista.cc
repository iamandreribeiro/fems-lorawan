/*
 * boa-vista.cc
 * Cenário LoRaWAN — Fazenda Boa Vista (80 ha) para o FEMS multiagente.
 * Baseado no complete-network-example do módulo signetlabdei/lorawan.
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/lorawan-module.h"

#include <sstream>

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("BoaVistaScenario");

int
main(int argc, char* argv[])
{
    uint32_t nDevices = 20;
    double radius = 500.0;
    double simTimeHours = 24.0;
    double appPeriodSeconds = 600.0;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nDevices", "Numero de end devices (agentes)", nDevices);
    cmd.AddValue("radius", "Raio da area (m)", radius);
    cmd.AddValue("simTimeHours", "Tempo simulado (h)", simTimeHours);
    cmd.AddValue("appPeriodSeconds", "Periodo de envio (s)", appPeriodSeconds);
    cmd.Parse(argc, argv);

    Time appStopTime = Hours(simTimeHours);

    Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel>();
    loss->SetPathLossExponent(3.76);
    loss->SetReference(1, 7.7);
    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();
    Ptr<LoraChannel> channel = CreateObject<LoraChannel>(loss, delay);

    LoraPhyHelper phyHelper;
    phyHelper.SetChannel(channel);
    LorawanMacHelper macHelper;
    LoraHelper helper;
    helper.EnablePacketTracking();

    Ptr<ListPositionAllocator> gwPos = CreateObject<ListPositionAllocator>();
    gwPos->Add(Vector(0.0, 0.0, 15.0));
    Ptr<UniformDiscPositionAllocator> edPos = CreateObject<UniformDiscPositionAllocator>();
    edPos->SetX(0.0);
    edPos->SetY(0.0);
    edPos->SetRho(radius);

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    NodeContainer endDevices;
    endDevices.Create(nDevices);
    mobility.SetPositionAllocator(edPos);
    mobility.Install(endDevices);

    phyHelper.SetDeviceType(LoraPhyHelper::ED);
    macHelper.SetDeviceType(LorawanMacHelper::ED_A);
    macHelper.SetRegion(LorawanMacHelper::EU);
    helper.Install(phyHelper, macHelper, endDevices);

    NodeContainer gateways;
    gateways.Create(1);
    mobility.SetPositionAllocator(gwPos);
    mobility.Install(gateways);

    phyHelper.SetDeviceType(LoraPhyHelper::GW);
    macHelper.SetDeviceType(LorawanMacHelper::GW);
    helper.Install(phyHelper, macHelper, gateways);

    macHelper.SetSpreadingFactorsUp(endDevices, gateways, channel);

    PeriodicSenderHelper appHelper;
    appHelper.SetPeriod(Seconds(appPeriodSeconds));
    appHelper.SetPacketSize(23);
    ApplicationContainer apps = appHelper.Install(endDevices);
    apps.Start(Seconds(0));
    apps.Stop(appStopTime);

    Ptr<Node> networkServer = CreateObject<Node>();
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    P2PGwRegistration_t gwRegistration;
    for (auto gw = gateways.Begin(); gw != gateways.End(); ++gw)
    {
        auto container = p2p.Install(networkServer, *gw);
        auto serverP2PNetDev = DynamicCast<PointToPointNetDevice>(container.Get(0));
        gwRegistration.emplace_back(serverP2PNetDev, *gw);
    }
    NetworkServerHelper nsHelper;
    nsHelper.SetGatewaysP2P(gwRegistration);
    nsHelper.SetEndDevices(endDevices);
    nsHelper.Install(networkServer);

    ForwarderHelper forwarderHelper;
    forwarderHelper.Install(gateways);

    Simulator::Stop(appStopTime + Hours(1));
    Simulator::Run();
    Simulator::Destroy();

    LoraPacketTracker& tracker = helper.GetPacketTracker();
    std::string counts = tracker.CountMacPacketsGlobally(Seconds(0), appStopTime + Hours(1));
    std::istringstream iss(counts);
    double sent = 0, received = 0;
    iss >> sent >> received;
    double pdr = (sent > 0) ? received / sent : 0.0;

    std::cout << nDevices << "," << sent << "," << received << "," << pdr << std::endl;

    return 0;
}
