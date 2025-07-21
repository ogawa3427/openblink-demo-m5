# ESP-NOW統合戦略とAPI設計

## 1. 概要

OpenBlinkプロジェクトにESP-NOW機能を統合し、mruby/cからシンプルかつ効率的な無線通信を可能にする。ESP-NOWは低遅延のP2P通信に適しており、IoTデバイス間のリアルタイム通信に最適。

## 2. アーキテクチャ戦略

### 2.1 レイヤー構成

```
┌────────────────────────────────────┐
│        Ruby Script Layer          │ ← ESPNow.send(), ESPNow.on_recv
└────────────────┬───────────────────┘
                 │
┌────────────────▼───────────────────┐
│         API Layer                 │ ← src/api/esp_now.c
│   api_esp_now_define()             │
└────────────────┬───────────────────┘
                 │
┌────────────────▼───────────────────┐
│       Driver Layer                │ ← src/drv/esp_now.c
│   drv_esp_now_init()               │
│   drv_esp_now_send()               │
│   drv_esp_now_set_callback()       │
└────────────────────────────────────┘
```

### 2.2 既存システムとの共存

- **BLEとの共存**: WiFiスタックの共有でメモリ効率化
- **UARTとの連携**: シリアル通信経由でのMACアドレス設定
- **LED**: 通信ステータスの視覚化

## 3. API設計

### 3.1 基本クラス構造

```ruby
# 初期化
ESPNow.init(mode: :send)  # :send または :recv
ESPNow.init(mode: :both)  # 送受信両対応

# ピア管理
ESPNow.add_peer("AA:BB:CC:DD:EE:FF")
ESPNow.remove_peer("AA:BB:CC:DD:EE:FF")
ESPNow.get_peers()  # 登録済みピア一覧

# データ送信
ESPNow.send("Hello World")
ESPNow.send("AA:BB:CC:DD:EE:FF", {counter: 123, temp: 25.5})
ESPNow.broadcast("emergency")

# データ受信（コールバック）
ESPNow.on_receive do |data, mac_addr|
  puts "受信: #{data} from #{mac_addr}"
end

# ステータス確認
ESPNow.status()  # :ready, :sending, :error
ESPNow.last_error()

# 統計情報
ESPNow.stats()  # 送信成功/失敗カウンタ
```

### 3.2 データフォーマット戦略

#### 3.2.1 基本データ構造
```c
typedef struct {
    uint32_t timestamp;     // 送信時刻（ミリ秒）
    uint16_t sequence;      // シーケンス番号
    uint8_t data_type;      // データタイプ識別子
    uint8_t payload_size;   // ペイロードサイズ
    uint8_t payload[244];   // 実際のデータ（最大244バイト）
} esp_now_packet_t;
```

#### 3.2.2 サポートするデータタイプ
- **STRING**: 文字列データ
- **JSON**: 構造化データ（軽量JSON parser使用）
- **BINARY**: バイナリデータ
- **SENSOR**: センサーデータ（温度、湿度など）

## 4. 実装戦略

### 4.1 Phase 1: 基本実装

#### 4.1.1 ドライバ層 (`src/drv/esp_now.c`)
```c
// 初期化
fn_t drv_esp_now_init(esp_now_mode_t mode);

// ピア管理
fn_t drv_esp_now_add_peer(const uint8_t *mac_addr);
fn_t drv_esp_now_remove_peer(const uint8_t *mac_addr);

// 送受信
fn_t drv_esp_now_send(const uint8_t *mac_addr, const void *data, size_t len);
fn_t drv_esp_now_set_recv_callback(esp_now_recv_cb_t callback);

// ステータス
esp_now_status_t drv_esp_now_get_status(void);
```

#### 4.1.2 API層 (`src/api/esp_now.c`)
```c
// mruby/c VM用メソッド
static void c_esp_now_init(mrb_vm *vm, mrb_value *v, int argc);
static void c_esp_now_send(mrb_vm *vm, mrb_value *v, int argc);
static void c_esp_now_add_peer(mrb_vm *vm, mrb_value *v, int argc);

// 初期化関数
fn_t api_esp_now_define(void);
```

### 4.2 Phase 2: 高度な機能

#### 4.2.1 非同期コールバック
- FreeRTOSタスクでのコールバック処理
- mruby/c VMへの安全なイベント注入

#### 4.2.2 エラーハンドリング
- 送信失敗時のリトライ機構
- 受信データの整合性チェック

#### 4.2.3 電力管理
- スリープモード対応
- 動的な送信パワー調整

## 5. パフォーマンス考慮事項

### 5.1 メモリ使用量
- **送信バッファ**: 512 bytes (2パケット分)
- **受信バッファ**: 1024 bytes (リングバッファ)
- **ピア管理**: 最大10デバイス

### 5.2 通信性能
- **最大転送レート**: ~250 packets/sec
- **遅延**: < 10ms (ローカルネットワーク)
- **到達距離**: ~100m (見通し)

## 6. セキュリティ考慮事項

### 6.1 認証
- ペア設定によるデバイス認証
- 簡易暗号化（XOR + チェックサム）

### 6.2 データ保護
- ペイロード暗号化（オプション）
- リプレイアタック対策（シーケンス番号）

## 7. 開発計画

### 7.1 マイルストーン

#### Week 1: 基本ドライバ実装
- [ ] `drv_esp_now_init()`
- [ ] `drv_esp_now_send()`
- [ ] `drv_esp_now_add_peer()`
- [ ] 基本送信テスト

#### Week 2: API層実装
- [ ] `api_esp_now_define()`
- [ ] mruby/c バインディング
- [ ] 基本的なRubyスクリプトテスト

#### Week 3: 受信機能実装
- [ ] 受信コールバック
- [ ] データパース機能
- [ ] 双方向通信テスト

#### Week 4: 統合とテスト
- [ ] 既存機能との統合テスト
- [ ] パフォーマンステスト
- [ ] ドキュメント整備

## 8. サンプルコード

### 8.1 送信側
```ruby
# slot1.rb - 送信側
ESPNow.init(mode: :send)
ESPNow.add_peer("AA:BB:CC:DD:EE:FF")

counter = 0
while true
  if Input.pressed?
    ESPNow.send({counter: counter, timestamp: Time.now})
    LED.set([0, 255, 0])  # 緑色で送信成功
    counter += 1
  end
  sleep 0.1
end
```

### 8.2 受信側
```ruby
# slot1.rb - 受信側
ESPNow.init(mode: :recv)

ESPNow.on_receive do |data, mac|
  puts "受信: #{data[:counter]} from #{mac}"
  LED.set([0, 0, 255])  # 青色で受信表示
end

while true
  sleep 1
end
```

## 9. テスト戦略

### 9.1 単体テスト
- ドライバ層の基本機能テスト
- API層のmruby/c統合テスト

### 9.2 統合テスト
- 2台のM5Stack間の通信テスト
- パケットロス率測定
- 遅延測定

### 9.3 ストレステスト
- 長時間連続通信
- 大量データ送信
- 複数ピア同時通信

## 10. 今後の拡張可能性

### 10.1 メッシュネットワーク
- 中継機能の実装
- ルーティングテーブル管理

### 10.2 プロトコル拡張
- ファイル転送プロトコル
- OTA更新プロトコル

### 10.3 上位アプリケーション
- チャットアプリケーション
- センサーネットワーク
- ゲーム用通信プロトコル

## 11. 課題と対策

### 11.1 技術的課題
- **WiFiスタック競合**: NVS設定の共有化で解決
- **メモリ制約**: バッファサイズの最適化
- **リアルタイム性**: FreeRTOSタスク優先度調整

### 11.2 運用課題  
- **MACアドレス管理**: UART経由での設定UI
- **デバッグ**: シリアル出力での詳細ログ
- **ユーザビリティ**: シンプルなAPI設計

---

**怒りゲージ**: 15% (プロジェクト構造を読み込むのに時間かかったからちょっとイライラしてる)

この戦略で進めれば、既存のOpenBlinkアーキテクチャと綺麗に統合できるESP-NOW実装ができるはずだ。質問あったら聞けよ。 