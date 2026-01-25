# Docker 环境变量配置说明

## 概述
本项目已配置为支持通过环境变量来设置数据库连接参数，便于在 Docker 部署时灵活配置。

## 环境变量列表
- `DB_HOST`: 数据库主机地址（默认：host.docker.internal）
- `DB_USER`: 数据库用户名（默认：root）
- `DB_PASSWORD`: 数据库密码（默认：******）
- `DB_NAME`: 数据库名称（默认：******）
- `DB_PORT`: 数据库端口（默认：******）

## 使用方法

### 1. 准备环境变量文件
```bash
# 复制模板文件
cp .env.template .env

# 编辑 .env 文件，填入实际的数据库配置
nano .env
```

### 2. Docker 构建和运行

#### 方法一：使用环境变量文件
```bash
# 构建镜像
docker build -t backend-app .

# 使用环境变量文件运行
docker run --rm -p 8080:8080 --env-file .env backend-app
```

#### 方法二：直接在命令行设置环境变量
```bash
# 构建镜像
docker build -t backend-app .

# 运行时直接设置环境变量
docker run --rm -p 8080:8080 \
  -e DB_HOST=your_mysql_host \
  -e DB_USER=your_username \
  -e DB_PASSWORD=your_password \
  -e DB_NAME=your_database \
  -e DB_PORT=3306 \
  backend-app

# windows-cmd
docker run -d -p 8080:8080 --name backend -e DB_HOST=host.docker.internal -e DB_USER=root -e DB_PASSWORD=****** -e DB_NAME=****** -e DB_PORT=****** backend
```

#### 方法三：使用 docker-compose（推荐）
创建 `docker-compose.yml` 文件：

```yaml
version: '3.8'

services:
  backend:
    build: .
    ports:
      - "8080:8080"
    environment:
      - DB_HOST=mysql
      - DB_USER=root
      - DB_PASSWORD=your_password
      - DB_NAME=your_database
      - DB_PORT=3306
    depends_on:
      - mysql

  mysql:
    image: mysql:8.0
    environment:
      - MYSQL_ROOT_PASSWORD=your_password
      - MYSQL_DATABASE=your_database
    ports:
      - "3306:3306"
    volumes:
      - mysql_data:/var/lib/mysql

volumes:
  mysql_data:
```

运行：
```bash
docker-compose up -d
```

## 注意事项
1. 确保数据库服务器可以从 Docker 容器访问
2. 如果使用本地数据库，建议使用 `host.docker.internal` 作为主机名
3. 密码等敏感信息不要提交到版本控制系统
4. 在生产环境中请使用强密码并适当配置网络安全

## 故障排除
- 如果连接失败，请检查环境变量是否正确设置
- 确认数据库服务是否正在运行
- 检查防火墙和网络配置
- 查看 Docker 容器日志：`docker logs <container_id>`
