# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Architecture Overview

This is a home Kubernetes cluster managed through GitOps using ArgoCD. The repository contains:

- **app-of-apps/**: ArgoCD Application manifests that define what gets deployed
- **manifests/**: Kubernetes manifests organized by service/component

The cluster uses an "App of Apps" pattern where ArgoCD applications in `app-of-apps/` reference paths in `manifests/` or external Helm charts.

## Key Components

### ArgoCD Applications
Each `.yml` file in `app-of-apps/` is an ArgoCD Application that either:
- Points to `manifests/<service>/` for custom Kubernetes resources
- Points to external Helm repositories for third-party applications

### Services Deployed
- **ArgoCD**: GitOps controller (self-managing)
- **Cloudflared**: Cloudflare tunnel for external access
- **Monitoring**: Prometheus + Grafana stack via Helm
- **Storage**: Longhorn distributed storage
- **Development**: VS Code server, PostgreSQL + pgAdmin

### Networking
- Cloudflared tunnel provides external access to services via `*.coil398.io` domains
- Services use ClusterIP and are exposed through the tunnel configuration
- LoadBalancer services use MetalLB for bare-metal load balancing

## Common Operations

### Deploying New Services
1. Create manifests in `manifests/<service-name>/`
2. Create ArgoCD Application in `app-of-apps/<service-name>.yml`
3. Commit and push - ArgoCD will sync automatically

### Modifying Existing Services
- Direct manifest changes: Edit files in `manifests/`
- Helm-based services: Modify values in the Application's `helm.values` section

### Repository Structure
- All ArgoCD Applications point to `repoURL: https://github.com/coil398/home-k8s`
- Target revision is typically `main`
- Automated sync is enabled for most applications

## Storage Classes
- `longhorn-r2`: Used for persistent storage (Longhorn with 2 replicas)