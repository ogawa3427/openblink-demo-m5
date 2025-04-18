# OpenBlink 実装詳細

このドキュメントでは、OpenBlinkの内部実装と拡張方法について詳細に説明します。

## 実装アーキテクチャ

OpenBlinkは、M5Stack向けのmruby/cベースのプログラミング環境です。主要コンポーネントとその関係は以下の通りです：

- mruby/c VM: Rubyコードの実行環境
- M5Unified: ハードウェア制御ライブラリ
- C/C++バインディング: Rubyとネイティブコードのブリッジ

## APIの拡張方法

新たなAPIを追加するには、既存の機能と同様のパターンに従う必要があります。

1. 新しい機能のためのヘッダーとCPPファイルを作成
2. c_m5u.cppの`init_c_m5u()`関数に新しいクラスの初期化関数を追加

例えば、センサー機能を追加する場合：

```cpp
// c_sensor.h
extern "C" {
    void class_sensor_init();
}

// c_sensor.cpp
#include <M5Unified.h>
#include "my_mrubydef.h"

#ifdef USE_SENSOR
#include "c_sensor.h"

static mrbc_class *sensor_class;

// センサークラスの実装
static void c_sensor_new(mrb_vm *vm, mrb_value *v, int argc) {
    v[0] = mrbc_instance_new(vm, v[0].cls, sizeof(void*));
    mrbc_instance_call_initialize(vm, v, argc);
}

// ...メソッド実装...

void class_sensor_init() {
    sensor_class = mrbc_define_class(0, "Sensor", mrbc_class_object);
    mrbc_define_method(0, sensor_class, "new", c_sensor_new);
    // ...メソッド定義...
}
#endif // USE_SENSOR
```

## システム構造

このファームウェアの基本構造：

- `main.c`: エントリーポイント、mruby/c VMの初期化と実行
- `src/m5u/`: M5Stackの機能のバインディング
- `src/api/`: 基本APIの定義
- `src/rb/`: Rubyコード
- `src/app/`: アプリケーション初期化
- `src/drv/`: デバイスドライバ層

## 詳細実装

より詳細な実装情報については、ソースコードのコメントを参照してください。 