# 使用 Ubuntu 22.04 作为基础镜像（稳定且支持 Qt6）
FROM ubuntu:22.04

# 关闭 tzdata 的交互式安装提示
ENV DEBIAN_FRONTEND=noninteractive

# 安装构建工具与所需 Qt6 模块
RUN apt-get update && apt-get install -y \
    build-essential \
    pkg-config \
    qt6-base-dev \
    qt6-network-dev \
    qt6-httpserver-dev \
    qt6-sql-dev \
    && rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /app

# 拷贝项目源码到容器
COPY . .

# 编译 Qt 程序（假设入口是 main.cpp）
RUN g++ main.cpp -o backend `pkg-config --cflags --libs Qt6Core Qt6Network Qt6HttpServer Qt6Sql`

# 暴露服务端口
EXPOSE 8080

# 启动服务
CMD ["./backend"]
