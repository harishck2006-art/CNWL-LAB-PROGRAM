#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/error-model.h"

using namespace ns3;

NodeContainer nodes;
NetDeviceContainer accessDevices;
NetDeviceContainer bottleneckDevices;
Ptr<FlowMonitor> monitor;

void NodeCreation()
{
    nodes.Create(10);

    InternetStackHelper stack;
    stack.Install(nodes);
}

void LinkCreation()
{
    PointToPointHelper p2p;

    p2p.SetDeviceAttribute(
        "DataRate", StringValue("10Mbps"));

    p2p.SetChannelAttribute(
        "Delay", StringValue("2ms"));

    accessDevices =
        p2p.Install(nodes.Get(0), nodes.Get(1));
}

void CongestionMaking()
{
    PointToPointHelper bottleneck;

    bottleneck.SetDeviceAttribute(
        "DataRate", StringValue("1Mbps"));

    bottleneck.SetChannelAttribute(
        "Delay", StringValue("10ms"));

    bottleneckDevices =
        bottleneck.Install(nodes.Get(2), nodes.Get(3));
}

void ErrorModeling()
{
    Ptr<RateErrorModel> error =
        CreateObject<RateErrorModel>();

    error->SetAttribute(
        "ErrorRate", DoubleValue(0.05));

    error->SetAttribute(
        "ErrorUnit",
        EnumValue(RateErrorModel::ERROR_UNIT_PACKET));

    bottleneckDevices.Get(1)->SetAttribute(
        "ReceiveErrorModel",
        PointerValue(error));
}

void OutputSummary()
{
    FlowMonitorHelper flowmon;

    monitor = flowmon.InstallAll();

    Simulator::Stop(Seconds(10));
    Simulator::Run();

    monitor->CheckForLostPackets();

    Simulator::Destroy();
}

int main()
{
    NodeCreation();
    LinkCreation();
    CongestionMaking();
    ErrorModeling();
    OutputSummary();

    return 0;
}
