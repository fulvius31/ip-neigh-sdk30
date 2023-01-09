# IP neighbour for Android with TargetSDK 30 and above

#### [View Releases and Changelogs](https://github.com/fulvius31/ip-neigh-sdk30/releases)

[![License: CC0-1.0](https://img.shields.io/badge/License-CC0_1.0-lightgrey.svg)](http://creativecommons.org/publicdomain/zero/1.0/)

---
# What this library does?

This library is born because Google has removed the possibility to use command like `ip neigh` for all the apps with targetSDK `30`.

More precisely apps cannot bind netlink socket when targeting Android API 30, then apps cannot retreive Arp table anymore.

With this library, you can easily do that!

# Screenshot
<img src="https://github.com/fulvius31/ip-neigh-sdk30/blob/main/ipneigh_screen.png" width="250" height="540">

# How to import

Step 1. Add the JitPack repository to your build file
```gradle
allprojects {
  repositories {
    ...
    maven { url 'https://jitpack.io' }
  }
}
```

Step 2. Add the dependency
```gradle
dependencies {
  ...
  implementation 'com.github.fulvius31:ip-neigh-sdk30:v0.0.3alpha'
}
```

# How to use

Use this library, is pretty easy. You have to call a method from the library that returns a String.

``` java
import it.alessangiorgi.ipneigh30.ArpNDK;

...

String arpTable = ArpNDK.getARP();
```

# Important Note

This library **does not** work with Android 13 devices if you use `targetSdk` equals to `32` .

## Apps Using this Library

- https://play.google.com/store/apps/details?id=com.sangiorgisrl.wifimanagertool
- https://play.google.com/store/apps/details?id=com.tester.wpswpatester
