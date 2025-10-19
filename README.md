AZ1UBALLのZMK用ドライバーです。

AZ1UBALLはパレットシステム様が販売している1U(16mm×16mm)サイズのトラックボールモジュールです。
I2C通信でボールの回転＋クリックを取得できます。

&nbsp;  
## インストール

「west.yml」に下記の内容を追加してください。

```yml
manifest:
  remotes:
    ...
    # START #####
    - name: kzyz
      url-base: https://github.com/kzyz
    # END #######
    ...
  projects:
    ...
    # START #####
    - name: zmk_az1uball_driver
      remote: kzyz
      revision: main
    # END #######
    ...
  self:
    path: config
```

&nbsp;  
使用している「〜.dtsi」または「〜.overlay」に必要な部分を追加してください。
（I2Cのピンは使用しているものあわせてください）

```dts
#include <dt-bindings/zmk/input_transform.h>

&pinctrl {
    /* configuration for i2c0 device, default state */
    i2c0_default: i2c0_default {
        group1 {
            psels = <NRF_PSEL(TWIM_SDA, 0, 4)>,
                <NRF_PSEL(TWIM_SCL, 0, 5)>;
        };
    };

    i2c0_sleep: i2c0_sleep {
        group1 {
            psels = <NRF_PSEL(TWIM_SDA, 0, 4)>,
                <NRF_PSEL(TWIM_SCL, 0, 5)>;
            low-power-enable;
        };
    };
};

&i2c0 {
    compatible = "nordic,nrf-twim"; // I2C controller instead of generic
    status = "okay";
    pinctrl-0 = <&i2c0_default>;
    pinctrl-1 = <&i2c0_sleep>;
    pinctrl-names = "default", "sleep";
    clock-frequency = <I2C_BITRATE_FAST>;
    
    trackball: trackball@a {
        compatible = "palette,az1uball";
        reg = <0x0A>;
        status = "okay";
    };
};

trackball_input_listener {
        compatible = "zmk,input-listener";
        device = <&trackball>;
        input-processors = <&zip_xy_scaler 4 1>;
};
```

&nbsp;  
「Kconfig.defconfig」に必要な部分を追加してください。

```conf
...
config ZMK_POINTING
default y

config AZ1UBALL
default y

CONFIG_I2C
default y
...
```

&nbsp;  
## カスタマイズ

Input Processorを使用してカーソルの移動速度の変更やスクロール用に変更、
オートマウスレイヤーを使用するなどのカスタマイズできます。  
Input Processorの詳しい日本語解説はkot149様の記事
「[ZMK Input Processorチートシート](https://zenn.dev/kot149/articles/zmk-input-processor-cheat-sheet)」
をご覧になっていただくのが一番わかりやすいと思います。