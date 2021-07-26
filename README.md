# AccelByteNetworkUtilities UE4 plugin #

AccelByteNetworkUtilities will provides network utilities in UE4. Currently it contains an Interactive Connectivity Establishment connection for multiplayer client - server games. This plugin is used together with the AccelByteOSS. This should be used as submodule of the OSS.

### ICE (Interactive Connectivity Establishment) ###

is a technique used in computer networking to find ways for two computers to talk to each other as directly as possible in peer-to-peer networking. This is most commonly used for interactive media such as Voice over Internet Protocol (VoIP), peer-to-peer communications, video, and instant messaging. In such applications, you want to avoid communicating through a central server (which would slow down communication, and be expensive), but direct communication between client applications on the Internet is very tricky due to network address translators (NATs), firewalls, and other network barriers.
Detail please check [https://en.wikipedia.org/wiki/Interactive_Connectivity_Establishment](https://en.wikipedia.org/wiki/Interactive_Connectivity_Establishment)

### How to use AccelByteNetworkUtilities ###

To use the ICE connection we need to use the IpNetDriverAccelByte. How to use it:
1. Update the DefaultEngine.ini
```
[/Script/AccelByteNetworkUtilities.IpNetDriverAccelByte]
NetConnectionClassName=AccelByteNetworkUtilities.IpConnectionAccelByte

[AccelByteNetworkUtilities]
TurnServerUrl="54.169.94.224"
TurnServerPort=3478
TurnServerUsername="accelbyte"
TurnServerPassword="accelbyte"
```

2. Edit the platform specific config ini file. The platform specific config is inside the platform folder name, example for Windows will be in ```Config/Windows/WindowsEngine.ini```
```
[/Script/Engine.GameEngine]
!NetDriverDefinitions=ClearArray
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="AccelByteNetworkUtilities.IpNetDriverAccelByte",DriverClassNameFallback="OnlineSubsystemUtils.IpNetDriver")
```
