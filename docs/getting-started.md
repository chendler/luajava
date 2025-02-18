# Getting Started

You may try things first [with an interactive console](./console.md).

## Getting It

::: tip
Go to [Initializer](#initializer) to get the required dependencies for your build system.
:::

The library consists of two parts:

1. The Java interface
2. The compiled native binaries

So you will need both to get LuaJava to work correctly. Basically, using Maven Central:

- The `groupId` is `party.iroiro.luajava`.
- The Java interface is `party.iroiro.luajava:luajava`.
- The Lua specific bridging artifacts are `lua5N` (`lua51` `lua52` ...) or `luajit`.
- The natives has `artifactId` like `lua5N-platform` (`lua51` `lua52` ...) or `luajit-platform`.

However, there are different native artifacts for different platforms, each with a different `classifier`:

- For desktop platforms, including Linux, Windows and macOS, on x64 or x32 or ARM(32/64), we provide an integrated
  artifact with classifier `natives-desktop`.
- For mobile devices:
    - iOS: An artifact with classifier `natives-ios`.
    - Android: Since there are different architectures for Android, you can choose from the following four according to
      your target devices. (I recommend you to just include all the four though.)
        - Artifact with classifier `natives-armeabi-v7a`.
        - Artifact with classifier `natives-arm64-v8a`.
        - Artifact with classifier `natives-x86`.
        - Artifact with classifier `natives-x86_64`.

### Initializer

You may get the required dependencies for your build system using the following simple form.

For mobile platforms, e.g. iOS and Android, you might need some more configuration
to get things work.

<Matrix/>

## iOS

(Work in progress.)

## Android

You can choose between the following two configurations. The former one uses a pre-bundled AAR archive while the latter
might give a little more flexibility.

### Using bundled AAR files

The AAR archive bundles native binaries for `armeabi-v7a` `arm64-v8a` `x86` and `x86_64`.

```groovy
ext {
    // You may replace `lua51` with `luajit` or other Lua versions
    lua = 'luajit'
    luaJavaVersion = '3.3.0'
}

dependencies {
    // other dependencies
    implementation "party.iroiro.luajava:${lua}:${luaJavaVersion}"
    runtimeOnly "party.iroiro.luajava:android:${luaJavaVersion}:${lua}@aar"
}
```

### Using a really lengthy configuration

```groovy
android {
    // other configurations
    
    sourceSets {
        main {
            jniLibs.srcDirs = ['libs']
        }
    }
}

ext {
    // You may replace `lua51` with `luajit` or other Lua versions
    lua = 'lua51'
    luajavaVersion = '3.3.0'
}

configurations { natives }

dependencies {
    // other dependencies

    implementation "party.iroiro.luajava:luajava:${luajavaVersion}"
    implementation "party.iroiro.luajava:${lua}:${luajavaVersion}"
    natives "party.iroiro.luajava:${lua}-platform:${luajavaVersion}:natives-armeabi-v7a"
    natives "party.iroiro.luajava:${lua}-platform:${luajavaVersion}:natives-arm64-v8a"
    natives "party.iroiro.luajava:${lua}-platform:${luajavaVersion}:natives-x86"
    natives "party.iroiro.luajava:${lua}-platform:${luajavaVersion}:natives-x86_64"
}

// Generated by gdx-setup:
// called every time gradle gets executed, takes the native dependencies of
// the natives configuration, and extracts them to the proper libs/ folders
// so they get packed with the APK.
task copyAndroidNatives {
    doFirst {
        file("libs/armeabi-v7a/").mkdirs()
        file("libs/arm64-v8a/").mkdirs()
        file("libs/x86_64/").mkdirs()
        file("libs/x86/").mkdirs()

        configurations.natives.copy().files.each { jar ->
            def outputDir = null
            if (jar.name.endsWith("natives-arm64-v8a.jar")) outputDir = file("libs/arm64-v8a")
            if (jar.name.endsWith("natives-armeabi-v7a.jar")) outputDir = file("libs/armeabi-v7a")
            if(jar.name.endsWith("natives-x86_64.jar")) outputDir = file("libs/x86_64")
            if(jar.name.endsWith("natives-x86.jar")) outputDir = file("libs/x86")
            if(outputDir != null) {
                copy {
                    from zipTree(jar)
                    into outputDir
                    include "*.so"
                }
            }
        }
    }
}

tasks.whenTaskAdded { packageTask ->
    if (packageTask.name.contains("merge") && packageTask.name.contains("JniLibFolders")) {
        packageTask.dependsOn 'copyAndroidNatives'
    }
}
```

