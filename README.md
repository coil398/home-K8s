# home-K8s

GitOps でホームラボ向けの Kubernetes クラスタを管理するためのリポジトリです。Argo CD の「App of Apps」パターンで、外部 Helm チャートと本リポジトリ内のマニフェストを組み合わせて各種コンポーネントをデプロイします。

## 概要
- GitOps 管理: Argo CD が `app-of-apps/` 内の Application を監視・同期
- マニフェスト配置: 個別リソースは `manifests/`、Helm は Application から参照
- 主な用途: ストレージ、監視、ネットワーク、開発環境、DB、トンネル等のホームクラスタ運用

## リポジトリ構成
- `app-of-apps/`: Argo CD Application 一式（各コンポーネントごと）
- `manifests/`: 追加の Kubernetes マニフェスト（Service/Secret/Config 等）
- `CLAUDE.md`: アーキテクチャの補足メモ

## 主なコンポーネント
- ストレージ: Longhorn（`longhorn-r2` StorageClass）
- 監視: kube-prometheus-stack（Prometheus/Grafana）、Loki、Grafana Alloy（ログ収集）
- ネットワーク: Ingress-NGINX、MetalLB（Bare-metal LB）
- 開発環境: code-server（VS Code）、JupyterLab
- データベース: CloudNativePG（PostgreSQL クラスタ）、pgAdmin、Redis（Sentinel 構成）、MongoDB + mongo-express、MinIO
- CI/Workflow: Argo Workflows、Argo Events
- アクセス: Cloudflared（Cloudflare Tunnel）、Kubernetes Dashboard、Harbor（コンテナレジストリ）

## 事前準備（前提）
- 稼働中の Kubernetes クラスタ（例: MicroK8s）。Longhorn 用にワーカに十分なローカルストレージが必要
- Argo CD がクラスタにインストール済みであること（本リポジトリでは Argo CD 本体の導入は行いません）
  - 参考: `kubectl create namespace argocd && kubectl apply -n argocd -f https://raw.githubusercontent.com/argoproj/argo-cd/stable/manifests/install.yaml`
- DNS/ドメインと Cloudflare アカウント（Cloudflared 用）。`*.coil398.io` はサンプルなので自環境のドメインに置換してください
- Bare-metal 環境の場合、MetalLB 用のアドレスプールを自環境の L2 ネットワークに合わせて設定

## ブートストラップ手順
1) Argo CD の準備
- Argo CD をインストールし、`argocd` Namespace が利用可能な状態にする

2) Cloudflared クレデンシャル Secret の作成（任意／Cloudflare Tunnel 利用時）
- トンネル作成後に得られる `<tunnel ID>.json` を Secret に保存
- 例:
  ```sh
  kubectl -n default create secret generic tunnel-credentials \
    --from-file=credentials.json=/path/to/<tunnel ID>.json
  ```
  設定ファイルは `manifests/cloudflared/cloudflared.yml` の `ConfigMap` を参照し、ホスト名や宛先 Service を自環境に合わせて変更可能です

3) 変数・値の見直し（推奨）
- ドメイン: `*.coil398.io` を自分のドメインへ置換（例: Grafana/Harbor/code-server など）
- ネットワーク: `manifests/metallb/metallb.yml` の `IPAddressPool`（例: `172.28.7.0-172.28.7.10`）を自ネットワークに合わせる
- VIP/エンドポイント: `manifests/vip/service.yml` の IP (`172.28.6.100`) 等を自環境に合わせる
- ストレージクラス: `longhorn-r2` を使用（Longhorn 導入後に自動作成）
- デフォルトパスワード: `manifests/*/secret.yml` や Helm values 内の例示値は本番利用前に必ず変更

4) Application の適用（ブートストラップ）
- まとめて作成:
  ```sh
  kubectl apply -f app-of-apps/
  ```
- Argo CD UI から各 Application の同期状況を確認し、必要に応じて Sync を実行

## 重要なパスと役割
- `app-of-apps/argocd.yml`: Argo CD 用の Service 等（Argo CD 本体のインストールではない点に注意）
- `app-of-apps/metallb.yml`: MetalLB 本体 + `manifests/metallb/` の設定適用
- `app-of-apps/kube-prometheus-stack.yml`: 監視スタック（Grafana 連携 Secret は `manifests/grafana/secret.yml`）
- `app-of-apps/longhorn.yml`: Longhorn（MicroK8s 環境向け `kubeletRootDir` を指定済）
- `app-of-apps/cloudflared.yml`: Cloudflare Tunnel（`tunnel-credentials` Secret 必須）
- `app-of-apps/storage.yml`: `manifests/storage/` の `longhorn-r2` StorageClass と共通 PVC 等

## よくあるカスタマイズ
- Ingress/外部公開:
  - 多くの Service は ClusterIP で、Cloudflared により外部公開
  - Ingress-NGINX を併用し、内部 Ingress でルーティング（例: `manifests/code-server/ingress.yml`）
- 監視連携:
  - 各 Application の `helm.values` で `serviceMonitor` を有効化し、`release: kube-prometheus-stack` ラベルで検出
- ログ収集:
  - Grafana Alloy が Pod ログを発見して Loki へ集約

## セキュリティ/運用上の注意
- 初期値のパスワードやアクセストークンは必ず変更し、Secret で安全に管理
- `manifests/` 配下の Secret は学習/デモ向けの例示です。本番では外部 Secret 管理（External Secrets/Sealed Secrets 等）を推奨
- MetalLB のアドレスプールや VIP の IP は自環境に合わせないと競合や不達の原因になります
- 破壊的変更を伴う場合は Argo CD の `Sync` 設定（`selfHeal/prune`）や `Replace` オプションに注意

## トラブルシューティング
- Application が `OutOfSync` のまま: Argo CD イベント/差分を確認し、CRD 未導入や RBAC 不足をチェック
- Pod が Pending: ストレージクラス/ノード容量/アフィニティ等を確認（Longhorn の健康状態も）
- 外部公開不可: Cloudflared Secret/Config、DNS、宛先 Service 名/ポート、証明書/TLS の設定を確認

## ライセンス
- 本リポジトリには明示的なライセンスファイルが含まれていません。利用ポリシーはリポジトリ所有者に従ってください

