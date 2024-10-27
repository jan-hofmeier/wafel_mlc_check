# wafel_trampoline_demo

This plugin hooks right after the mlc mount and tries to read every file on the the mlc. Errors are locked to the `mlc_check.log` on the SD. 

During the check the LED will blink. As long as no errors were found it will blink blue, once errors are detected it will blink yellow. If a fatal problem occured the LED will blink red.
Once finsished it will set the LED to blue and continue the boot normally

## How to use

- Copy the `wafel_mlc_check.ipx` to `/wiiu/ios_plugins`

## Building

```bash
export STROOPWAFEL_ROOT=/path/too/stroopwafel-repo
make
```

## Thanks

- [shinyquagsire23](https://github.com/shinyquagsire23)
