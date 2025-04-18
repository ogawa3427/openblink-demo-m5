# OpenBlink API リファレンス

このドキュメントでは、OpenBlinkで利用可能なAPI機能について説明します。

## M5クラス

M5Stackの基本機能にアクセスするためのクラスです。

```ruby
# 初期化
M5.begin()

# ディスプレイ関連
M5.update()
M5.clear()
```

## LED操作

LEDを制御するためのメソッドです。

```ruby
# LEDを点灯
LED.on()

# LEDを消灯
LED.off()
```

## 入力

ボタン入力を検出するためのメソッドです。

```ruby
# ボタン入力の検出
if Input.a_pressed?
  # ボタンAが押された時の処理
end
```

## その他の機能

詳細は[実装詳細](./implementation_details.md)を参照してください。 