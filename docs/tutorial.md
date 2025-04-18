# OpenBlink チュートリアル

このチュートリアルでは、OpenBlinkを使ったプログラミングの基礎を説明します。

## 基本的なプログラム構造

OpenBlinkのプログラムは、mrubyベースのRubyコードで記述します。基本的な構造は以下の通りです：

```ruby
# 初期化処理
def setup
  M5.begin
end

# メインループ
def loop
  if Input.a_pressed?
    LED.on
  else
    LED.off
  end
  
  M5.update
end
```

## 簡単なLEDプログラム

以下は、ボタンを押すとLEDが点灯する簡単なプログラム例です：

```ruby
# LEDを制御する簡単なプログラム
def setup
  M5.begin
  puts "LED Control Program"
end

def loop
  if Input.a_pressed?
    LED.on
    puts "LED ON"
  elsif Input.b_pressed?
    LED.off
    puts "LED OFF"
  end
  
  sleep 0.1
  M5.update
end
```

## ディスプレイへの描画

ディスプレイに文字や図形を描画する例：

```ruby
def setup
  M5.begin
  M5.clear
  
  # テキスト表示
  M5.print("Hello, OpenBlink!", 10, 10)
  
  # 図形描画
  M5.rect(50, 50, 100, 100)
  
  M5.update
end
```

## 次のステップ

より詳細な機能については、[APIリファレンス](./api_reference.md)を参照してください。 