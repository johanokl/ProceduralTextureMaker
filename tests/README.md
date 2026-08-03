# Test Suite

Configure and run the required integration tests with:

```sh
cmake --preset debug -DBUILD_TESTING=ON
cmake --build --preset debug
ctest --preset debug
```

The tests use temporary directories and isolated settings. GUI-dependent rendering runs with
`QT_QPA_PLATFORM=offscreen` under CTest. No test may read or write the normal application settings,
clipboard, or home-directory files.

## Rendering goldens

The persistence test records SHA-256 digests of raw `TexturePixel` RGBA bytes. PNG file bytes are
not hashed because encoder output can change without changing decoded pixels.

`minimal-fill.txl` is the portable exact fixture. The tracked example hashes characterize the
canonical Windows UCRT64/Qt 6 build; platform-sensitive font rasterization, antialiasing, and
floating-point behavior mean a new platform must receive a separately reviewed baseline.

Golden values must never be updated automatically. For an intentional rendering change:

1. run only `ProjectFileServiceTest::rendersStableRawHashes` on the old and new code;
2. export reference PNGs and inspect a decoded pixel difference;
3. record the reason for the behavioral change;
4. update the test in a change separate from mechanical refactoring.

## Concurrency tests

The render tests coordinate with semaphores and Qt signals. Timeouts are deadlock guards,
not ordering mechanisms. Stress the render-labelled tests before concurrency refactors:

```sh
ctest --preset debug -L render --repeat until-fail:50
```
