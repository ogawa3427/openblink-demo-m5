# ESP-NOW実装ガイド

## 概要

OpenBlinkプロジェクトにESP-NOW機能を統合し、M5Stack間でのリアルタイム無線通信を可能にしました。ESP-NOWは低遅延P2P通信に適しており、IoTデバイス間の即座のデータ交換に最適です。

## 実装ファイル

### ドライバー層
- `src/drv/blink_esp_now.h` - ESP-NOWドライバーのヘッダー
- `src/drv/blink_esp_now.c` - ESP-NOWドライバーの実装

### API層
- `src/api/esp_now_api.h` - mruby/c用API のヘッダー
- `src/api/esp_now_api.c` - mruby/c用APIの実装

### サンプルコード
- `sample_rb/esp_now_sender.rb` - 送信側サンプル
- `sample_rb/esp_now_receiver.rb` - 受信側サンプル

## Ruby API仕様

### 初期化
```ruby
ESPNow.init(:send)   # 送信専用モード
ESPNow.init(:recv)   # 受信専用モード  
ESPNow.init(:both)   # 送受信両対応モード
```

### ピア管理
```ruby
ESPNow.add_peer("AA:BB:CC:DD:EE:FF")    # ピア追加
ESPNow.remove_peer("AA:BB:CC:DD:EE:FF") # ピア削除
```

### データ送受信
```ruby
ESPNow.send(123)                    # 整数値を送信
counter = ESPNow.received_counter   # 受信カウンター取得
current = ESPNow.counter            # 送信カウンター取得
```

### ステータス確認
```ruby
status = ESPNow.status  # :ready, :sending, :error のいずれかを返す
```

### 終了処理
```ruby
ESPNow.deinit  # ESP-NOWを終了
```

## データ構造

ESP-NOWで送信されるデータは以下の構造になっています：

```c
typedef struct {
    int counter_value;       // カウンター値
    uint32_t timestamp;      // タイムスタンプ（ミリ秒）
} esp_now_data_t;
```

## 使用例

### 送信側
```ruby
# 初期化
ESPNow.init(:send)
ESPNow.add_peer("FF:FF:FF:FF:FF:FF")  # ブロードキャスト

counter = 0
loop do
  if Input.pressed?
    if ESPNow.send(counter)
      puts "Sent: #{counter}"
      LED.set([0, 255, 0])  # 緑色で成功表示
      counter += 1
    end
    sleep 0.3
  end
  sleep 0.1
end
```

### 受信側
```ruby
# 初期化
ESPNow.init(:recv)

last_received = -1
loop do
  current = ESPNow.received_counter
  if current != last_received
    puts "Received: #{current}"
    LED.set([0, 0, 255])  # 青色で受信表示
    last_received = current
  end
  sleep 0.1
end
```

## ビルド設定

ESP-NOWを使用するには、以下の設定が必要です：

### platformio.ini
```ini
[env:m5stack-atom]
board = m5stack-atom
framework = espidf
build_flags = 
  -DESP_NOW_ENABLED=1
lib_deps = 
  m5stack/M5Unified
```

### CMakeLists.txt への追加
```cmake
# ESP-NOWドライバーの追加
target_sources(${COMPONENT_LIB} PRIVATE
  "drv/blink_esp_now.c"
  "api/esp_now_api.c"
)
```

## パフォーマンス特性

- **最大転送レート**: ~250 packets/sec
- **遅延**: < 10ms (ローカルネットワーク)
- **到達距離**: ~100m (見通し)
- **メモリ使用量**: 
  - 送信バッファ: 512 bytes
  - 受信バッファ: 1024 bytes

## 注意事項

### WiFiスタックとの共存
ESP-NOWはWiFiスタックを使用するため、他のWiFi機能（BLE含む）との併用時はメモリ使用量に注意が必要です。

### MACアドレス設定
デフォルトではブロードキャスト（FF:FF:FF:FF:FF:FF）を使用しますが、特定のデバイス間通信では実際のMACアドレスを指定してください。

### エラーハンドリング
通信エラーは`ESPNow.status`で確認できます。`:error`状態の場合は再初期化を推奨します。

## デバッグ方法

### シリアル出力
ESP-NOWの動作ログはシリアル出力で確認できます：
```
I (12345) BLINK_ESP_NOW: ESP-NOW initialized successfully
I (12678) BLINK_ESP_NOW: Sending counter: 123
I (12890) BLINK_ESP_NOW: Send Success
```

### LEDでの状態表示
サンプルコードでは以下の色で状態を表示：
- 緑色: 送信成功
- 赤色: 送信失敗
- 青色: 受信
- 黄色: エラー状態

## トラブルシューティング

### 通信できない場合
1. 両デバイスが同じチャンネルにいるか確認
2. MACアドレスが正しく設定されているか確認
3. WiFiスタックが正常に初期化されているか確認

### メモリ不足の場合
1. 他のWiFi機能を無効化
2. ヒープサイズを増加
3. 受信バッファサイズを調整

## 今後の拡張予定

- メッシュネットワーク対応
- より複雑なデータ構造の送信
- 暗号化機能の追加
- ファイル転送プロトコル

---

**実装日**: 2025年1月
**対応デバイス**: M5Stack Atom, M5Stack StampS3 