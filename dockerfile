# 使用 Ubuntu 24.04 作为基础镜像
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# 1️⃣ 修复证书 + 替换为清华源
RUN apt-get update && apt-get install -y ca-certificates apt-transport-https gnupg && \
    echo "deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ noble main restricted universe multiverse\n\
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ noble-updates main restricted universe multiverse\n\
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ noble-backports main restricted universe multiverse\n\
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ noble-security main restricted universe multiverse" > /etc/apt/sources.list && \
    apt-get update

# 2️⃣ 安装基础依赖和 Qt6 开发组件
RUN apt-get install -y \
    build-essential \
    cmake \
    git \
    qt6-base-dev \
    qt6-base-private-dev \
    qt6-tools-dev \
    qt6-declarative-dev \
    qt6-websockets-dev \
    qt6-networkauth-dev \
    qt6-httpserver-dev \
    qt6-connectivity-dev \
    qt6-svg-dev \
    && rm -rf /var/lib/apt/lists/*

# 4️⃣ 拷贝你的源码
WORKDIR /app
COPY . .

# 5️⃣ 构建你的 Qt 项目
RUN cmake -B build -S . && cmake --build build -j$(nproc)

# 6️⃣ 开放端口
EXPOSE 8080

# 7️⃣ 启动命令
CMD ["./build/backend"]
