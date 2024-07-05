# Changelog

All notable changes to this project will be documented in this file. See [standard-version](https://github.com/conventional-changelog/standard-version) for commit guidelines.

### [4.3.4](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.3.4%0D4.3.3) (2024-07-05)


### Bug Fixes

* **turn:** update get closest turn server using the new one and get latencies first before send it to metrics endpoint ([581cbe1](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/581cbe1db5c3b28102b35f45a572f566078fe154))

### [4.3.3](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.3.3%0D4.3.2) (2024-05-22)


### Bug Fixes

* change signaling object using smart pointer ([2063ef6](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/2063ef62abb899c7bb5afde122bb979b7706b1b0))

### [4.3.2](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.3.2%0D4.3.1) (2024-03-24)


### Bug Fixes

* **setting:** make sure all overriden setting goes to accelbyte config injection as well. ([c5babe0](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/c5babe0afff2b6fbd7248d21d15dbdf7d275307a))

### [4.3.1](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.3.1%0D4.3.0) (2024-03-08)


### Bug Fixes

* linker error in UE4.27 linux ([d608f5c](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/d608f5c0beab6f1388eed9306ba725df99f53893))
* remove all tickers in AccelByteJuice in destructor to avoid crash ([2b37886](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/2b37886b92795031823886c50ee5c085cc9e8864))

## [4.3.0](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.3.0%0D4.2.7) (2024-01-18)


### Features

* add reconnection support for libjuice agent in case of network switching ([8ca8af2](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/8ca8af2a3294ec92635f3f173342eb8ebf84f08a))


### Bug Fixes

* incorrect Ticker Delegate used in AccelByteJuice ([32416f7](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/32416f72b1ae0eb6650222cf1a0ad348c212c3b5))

### [4.2.7](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.2.7%0D4.2.6) (2023-10-30)


### Bug Fixes

* Incorrect include paths ([0c2adec](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/0c2adec0fa14224db8951f72e17fe68f3bc7b776))

### [4.2.6](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.2.6%0D4.2.5) (2023-10-12)


### Bug Fixes

* **UE:** allow compilation on UE5.2 ([5e985df](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/5e985df4be4fe7e2124c937ba8ce530118f72bb5))
* **UE:** allow compilation on UE5.2 ([3c901a6](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/3c901a678896c35d488fbf14dbadcb980c372c4e))

### [4.2.5](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.2.5%0D4.2.4) (2023-09-25)

### [4.2.4](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.2.4%0D4.2.3) (2023-09-11)


### Bug Fixes

* deprecation warning for IWYU support in Unreal Engine 5.2 ([85a642d](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/85a642d064e6184d850b65b6091d87f6c7473c62))
* error when compiling without libjuice ([cdfcb82](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/cdfcb821aa776af161e14ab4b366192222ee4a3c))

### [4.2.3](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.2.3%0D4.2.2) (2023-09-08)


### Bug Fixes

* deprecation warning for IWYU support in Unreal Engine 5.2 ([7da7349](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/7da734971cf9a3a1cc820a1a62327d1ee32ef9bb))

### [4.2.2](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.2.2%0D4.2.1) (2023-08-28)


### Bug Fixes

* remove 'using namespace' in public header ([c721b52](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/c721b52a85a7a653e7ab71e5e76cff81c37d157d))

### [4.2.1](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.2.1%0D4.2.0) (2023-08-25)


### Bug Fixes

* missing headers on several files using Unreal Engine 5.2 ([c4065c7](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/c4065c7700492929b512506d8fcb2420eb865598))

## [4.2.0](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.2.0%0D4.1.2) (2023-07-31)


### Features

* redirect readme to docs portal ([8553f72](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/8553f724582ac2c6d14f904a5f51f6324711db7d))


### Bug Fixes

* initialize USTRUCT member to default value ([b7fd976](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/b7fd9767992834a2e5c80b67cb23d937f692f701))

### [4.1.2](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.1.2%0D4.1.1) (2023-07-17)


### Bug Fixes

* initialize USTRUCT member to default value ([92ba9bb](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/92ba9bb87f53a12a5ad5ab7cebd02e9cdf8f0f03))

### [4.1.1](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.1.1%0D4.1.0) (2023-07-03)


### Bug Fixes

* error access TMap from multi thread ([e5ecf46](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/e5ecf46c169f4869f04fddc41134dce9be9ec5cb))

## [4.1.0](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.1.0%0D4.0.3) (2023-06-19)


### Features

* update TypeHash implementation for IpAddressAccelByte to support Unreal Engine 5.2 ([d3d280c](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/d3d280c254ad3efb6b5128503e1f2858d013ae13))

### [4.0.3](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.0.3%0D4.0.2) (2023-06-14)


### Bug Fixes

* error access TMap from multi thread ([856b0d4](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/856b0d418b57df4cdb4d32323f30106f0e4e41b3))

### [4.0.2](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.0.2%0D4.0.1) (2023-06-05)

### [4.0.1](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.0.1%0D4.0.0) (2023-05-09)


### Bug Fixes

* missing include headers when the project doesn't use shared precompiled headers ([9a5b32f](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/9a5b32f51eeedbba6660c42a5f2743c22086eaf3))

## [4.0.0](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/4.0.0%0D3.0.0) (2023-04-26)


### ⚠ BREAKING CHANGES

* p2p connection using channel to connect each other now. This will not compatible with previous network utilities. The breaking changes only for client that uses Network Utilities directly.

* chore: remove unused code

* chore: use struct instead of json object

* chore: remove unused code

* chore: update versioning


Approved-by: Rifqi Dewangga
Approved-by: Wijanarko Sukma Pamungkas

* Merged in feature/AR-5427-reactivep2punreal-make-network-m (pull request #41) ([2fc48d4](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/2fc48d4ad04e2e785f41e28fca4f0e42327e66d7)), closes [#41](https://accelbyte.atlassian.net/browse/41)

## [3.0.0](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/3.0.0%0D2.10.0) (2023-02-28)


### ⚠ BREAKING CHANGES

* EAccelByteP2PConnectionStatus and EAccelBytePeerStatus changed from raw enum tu UENUM() enum class

### Bug Fixes

* enclose EAccelByteP2PConnectionStatus and EAccelBytePeerStatus in namespace and change to enum class ([31fb84f](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/31fb84fa54a84dc87f06779545600524d74ac54c))

## [2.10.0](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/2.10.0%0D2.9.0) (2023-02-13)


### Features

* add OnICERequestConnectFinished delegate and deprecate OnICEConnected delegate ([3528266](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/35282664575c3b00d231946480d2c84f316effac))
* check if peer still hosting before initiate connection ([7b78465](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/7b78465fe112695135576350a4c06b5df186af24))

## [2.9.0](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/2.9.0%0D2.8.0) (2023-01-30)

## [2.8.0](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/2.8.0%0D2.7.0) (2022-12-05)


### Features

* update implementation to support Unreal Engine 5.1 ([b19f1ad](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/b19f1adb5a4daa670774586da26eb0359feb9265))

## [2.7.0](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/2.7.0%0D2.6.0) (2022-11-07)


### Features

* implement request credential for turn server auth ([202a293](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/202a29360be33ae171176e35deac10eab9f0c179))


### Bug Fixes

* juice callback flow ([f62510a](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/f62510aa5754cb120950ea55d6a3a4f15fe25ef0))
* juice PS failed name ip resolution ([f27f833](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/f27f83338540e19bed23c25bd6b1a2d78fe6a656))

## [2.6.0](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/2.6.0%0D2.5.2) (2022-10-24)


### Features

* implement request credential for turn server auth ([fdce34c](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/fdce34c86f0efe339182146feab4422c3c6d37b7))


### Bug Fixes

* add some missing headers ([2b1dc97](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/2b1dc97b52dcf6729a406b4ddfbd1277cc3984c9))
* so file linked to versioned library ([3d7aa63](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/3d7aa63171d8ab74516d01ded966161e18d2fe22))

### [2.5.2](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/2.5.2%0D2.5.1) (2022-10-10)

### [2.5.1](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/2.5.1%0D2.5.0) (2022-09-30)


### Bug Fixes

* so file linked to versioned library ([24783fa](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/24783fa7507b6281fae8b7cd0d1133cd2d6ff7e5))

## [2.5.0](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/2.5.0%0D2.4.1) (2022-09-27)


### Features

* add OnRTCClosed delegate bringback from eville ([1e2bbd6](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/commits/1e2bbd600d0aa89284116b3c9560dd80184b2f22))

### [2.4.1](https://bitbucket.org/accelbyte/justice-ue4-network-utilities-plugin/branches/compare/2.4.1%0D2.4.0) (2022-09-12)
