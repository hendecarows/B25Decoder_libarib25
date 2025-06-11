# B25Decoder_libarib25

<https://github.com/tsukumijima/Multi2Dec>

Linux版[EDCB][link_edcb]で動作する`B25Decoder.so`を使用して下さい。

---

B25Decoder.dllと互換インターフェースを持つLinux版B25Decoder.soです。

`/usr/local/lib/edcb/B25Decoder.so`として配置すれば、[EDCB][link_edcb]のB25デコード処理が有効になります。
なお、動作には[stz2012版libarib25][link_arib25]が必要で、デコード処理に関係する最低限の部分だけしか実装していません。

## インストール

```ShellSession
git clone https://github.com/hendecarows/B25Decoder_libarib25.git
cd B25Decoder_libarib25
mkdir build
cd build
cmake ..
make
sudo make install
```

## ライセンス

GNU General Public License version 2.0

[link_edcb]: https://github.com/xtne6f/EDCB
[link_arib25]: https://github.com/stz2012/libarib25
