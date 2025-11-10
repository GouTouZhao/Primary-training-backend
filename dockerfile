# 使用 Ubuntu 24.04 作为基础镜像
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# 1️⃣ 修复证书 + 启用清华源（含 deb-src）
RUN apt-get update && apt-get install -y ca-certificates apt-transport-https gnupg && \
    echo "deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ noble main restricted universe multiverse\n\
deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ noble main restricted universe multiverse\n\
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ noble-updates main restricted universe multiverse\n\
deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ noble-updates main restricted universe multiverse\n\
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ noble-backports main restricted universe multiverse\n\
deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ noble-backports main restricted universe multiverse\n\
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ noble-security main restricted universe multiverse\n\
deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ noble-security main restricted universe multiverse" > /etc/apt/sources.list && \
    apt-get update

RUN apt-get install -y \
    build-essential cmake git clang \
    libc++-dev libc++abi-dev \
    ca-certificates apt-transport-https gnupg curl
ENV CXX=clang++
ENV CC=clang

# 2️⃣ 安装基础依赖和 Qt6 开发组件
RUN apt-get install -y \
    build-essential \
    cmake \
    git \
    qt6-base-dev \
    qt6-base-private-dev \
    qt6-tools-dev \
    qt6-base-dev-tools \ 
    qt6-declarative-dev \
    qt6-websockets-dev \
    qt6-networkauth-dev \
    qt6-httpserver-dev \
    qt6-connectivity-dev \
    qt6-svg-dev \
    libmariadb-dev \
    && rm -rf /var/lib/apt/lists/*

# 3️⃣ 构建 Qt MySQL 驱动（Qt6 正确方式）
RUN git clone --depth=1 https://github.com/qt/qtbase.git -b 6.4.2 && \
    cd qtbase/src/plugins/sqldrivers && \
    cmake . -DQT_BUILD_EXAMPLES=OFF \
             -DQT_BUILD_TESTS=OFF \
             -DWITH_MYSQL=ON \
             -DMYSQL_INCLUDE_DIR=/usr/include/mariadb \
             -DMYSQL_LIBRARY=/usr/lib/x86_64-linux-gnu/libmariadb.so \
             -DCMAKE_INSTALL_PREFIX=/usr/lib/x86_64-linux-gnu/qt6 && \
    cmake --build . --target qsqlmysql --parallel $(nproc) && \
    find . -name "libqsqlmysql.so" && \
    cp ./mysql/../lib/x86_64-linux-gnu/qt6/plugins/sqldrivers/libqsqlmysql.so /usr/lib/x86_64-linux-gnu/qt6/plugins/sqldrivers/


# 4️⃣ 拷贝你的源码
WORKDIR /app
COPY . .

# 5️⃣ 构建你的 Qt 项目
RUN cmake -B build -S . && cmake --build build -j$(nproc)


# 6️⃣ 开放端口
EXPOSE 8080

# 7️⃣ 启动命令
CMD ["./build/backend"]
