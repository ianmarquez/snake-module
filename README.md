# Snake Dongle Module 🐍

Snake Dongle is a compact, highly customizable ZMK-powered dongle that features a Snake‑game-style animation and optional sound effects.
Complete documentation [here](https://github.com/joaopedropio/snake-dongle).
Click [here](https://www.youtube.com/watch?v=xdSUZYLVVY0) to watch a demo.

<img src="https://i.imgur.com/5ogG2z9.jpeg"/> 

## CPI status widget

Set any `CONFIG_INFO_SLOT_*` option to `"cpi"` to show the configured pointing
device CPI on the status screen. When
[`zmk-behavior-sensor-attr-cycle`](https://github.com/george-norton/zmk-behavior-sensor-attr-cycle)
is present, the widget follows the behavior's selected value dynamically,
including a value restored from settings.

```conf
CONFIG_INFO_SLOT_4="cpi"
```

The first enabled `zmk,behavior-sensor-attr-cycle` node is used by default. If
the keymap has more than one attribute cycler, select the CPI cycler explicitly:

```dts
/ {
    chosen {
        zmk,cpi-cycle = &cpi_cycle;
    };
};
```

`CONFIG_CPI_VALUE` remains available as a static fallback when the cycle
behavior module is not included.
