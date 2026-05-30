// swift-tools-version: 6.0
//
// T-0011: Swift Interop Prototype
//
// This package proves the Swift/C++ direct interop boundary. It wraps ScriviCore
// (a C++23 static library built by CMake) with a thin Swift layer.
//
// Build requirements:
//   - macOS 26.0+ (SDK: MacOSX26.2)
//   - Xcode 26+
//   - ScriviCore static library must be built first:
//       cmake -S ../.. -B ../../build -DSCRIVI_BUILD_TESTS=OFF
//       cmake --build ../../build --parallel
//
// Then build this package from platforms/apple/:
//   swift build
//
// Or run tests:
//   swift test

import PackageDescription

// Paths to ScriviCore artifacts.
// ScriviCore must be built with CMake before building this package.
let repoRoot              = "/Users/justgus/Xcode-Projects/Scrivi"
let scriviCoreLib         = "\(repoRoot)/build/ScriviCore/libScriviCore.a"
let scriviCoreIncludes    = "\(repoRoot)/ScriviCore/include"
let scriviCoreSrcIncludes = "\(repoRoot)/ScriviCore/src"

let package = Package(
    name: "Scrivi",
    platforms: [
        .macOS("26.0"),
    ],
    products: [
        .library(name: "Scrivi", targets: ["Scrivi"]),
    ],
    targets: [
        // C++ adapter target — links against the pre-built ScriviCore static library.
        // Swift/C++ interop bridge is defined by the module.modulemap in this target's sources.
        .target(
            name: "ScriviCoreAdapter",
            path: "Sources/ScriviCoreAdapter",
            sources: ["ScriviCoreAdapter.cpp", "KeychainSecureStore.cpp"],
            publicHeadersPath: ".",
            cxxSettings: [
                .unsafeFlags([
                    "-std=c++2b",
                    // ScriviCore public headers
                    "-I", scriviCoreIncludes,
                    // ScriviCore private src headers (LocalFileSystem.hpp, SystemUUIDProvider.hpp, etc.)
                    "-I", scriviCoreSrcIncludes,
                ]),
            ],
            linkerSettings: [
                .unsafeFlags([scriviCoreLib]),
                .linkedFramework("Security"),
            ]
        ),

        // Swift wrapper target — imports ScriviCoreAdapter via Swift/C++ direct interop.
        .target(
            name: "Scrivi",
            dependencies: ["ScriviCoreAdapter"],
            path: "Sources/Scrivi",
            swiftSettings: [
                .interoperabilityMode(.Cxx),
                // Pass ScriviCore include paths to the Clang frontend used during
                // Swift/C++ module import so the header can resolve its #includes.
                .unsafeFlags([
                    "-Xcc", "-I\(scriviCoreIncludes)",
                    "-Xcc", "-I\(scriviCoreSrcIncludes)",
                ]),
            ]
        ),

        // Test target — exercises the full end-to-end path.
        .testTarget(
            name: "ScriviInteropTests",
            dependencies: ["Scrivi"],
            path: "Tests/ScriviInteropTests",
            swiftSettings: [
                .interoperabilityMode(.Cxx),
                .unsafeFlags([
                    "-Xcc", "-I\(scriviCoreIncludes)",
                    "-Xcc", "-I\(scriviCoreSrcIncludes)",
                ]),
            ]
        ),
    ],
    cxxLanguageStandard: .cxx2b
)
