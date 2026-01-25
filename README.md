# Primary Training Backend

## 前端
https://github.com/agsd-agsd/Flight-Management-System-FrontEnd.git

一个基于 Qt6/C++ 的高性能后端服务，提供用户管理、票务系统、货币和积分系统等功能。

## 技术栈

- **语言**: C++17
- **框架**: Qt6 (Core, Network, Sql, HttpServer, WebSockets)
- **数据库**: MySQL/MariaDB
- **构建工具**: CMake
- **容器化**: Docker

## 主要功能

- 用户注册、登录、信息管理
- 管理员用户管理功能
- 票务系统（购票、退票、订单管理）
- 货币和积分系统
- 星级评价系统
- RESTful API 接口
- CORS 跨域支持

## 快速开始

### 方式一：直接运行

#### 环境要求
- Qt6.4.0+
- MySQL/MariaDB
- CMake 3.16+
- C++17 编译器

#### 编译运行
## 文件夹含mysql文件，记得先部署数据库
```bash
# 克隆项目
git clone https://github.com/GouTouZhao/Primary-training-backend.git
cd backend

# 配置数据库连接
export DB_HOST=localhost
export DB_USER=root
export DB_PASSWORD=your_password
export DB_NAME=your_database
export DB_PORT=3306

# 构建
cmake -B build -S .
cmake --build build

# 运行
./build/backend
```

服务将在 `http://localhost:8080` 启动



### 方式二：Docker 运行

## Docker Hub

镜像地址：https://hub.docker.com/repository/docker/goutouz/backend/general

#### 使用预构建镜像
```bash
# 拉取镜像
docker pull goutouz/backend

# 运行容器
docker run -d -p 8080:8080 --name backend \
  -e DB_HOST=host.docker.internal \
  -e DB_USER=root \
  -e DB_PASSWORD=your_password \
  -e DB_NAME=your_database \
  -e DB_PORT=3306 \
  goutouz/backend
```

#### 本地构建运行
```bash
# 构建镜像
docker build -t backend-app .

# 运行容器
docker run -d -p 8080:8080 --name backend \
  -e DB_HOST=host.docker.internal \
  -e DB_USER=root \
  -e DB_PASSWORD=your_password \
  -e DB_NAME=your_database \
  -e DB_PORT=3306 \
  backend-app
```


## 环境变量配置

| 变量名 | 描述 | 默认值 |
|--------|------|--------|
| `DB_HOST` | 数据库主机地址 | host.docker.internal |
| `DB_USER` | 数据库用户名 | root |
| `DB_PASSWORD` | 数据库密码 | ****** |
| `DB_NAME` | 数据库名称 | ****** |
| `DB_PORT` | 数据库端口 | ****** |

## API 接口

### 用户相关
- `POST /Register` - 用户注册
- `POST /Login` - 用户登录
- `POST /Info` - 获取用户信息
- `POST /UpdateProfileColor` - 更新头像颜色
- `POST /UpdateUsername` - 更新用户名

### 管理员相关
- `POST /AdminPasswordVerify` - 管理员密码验证
- `POST /AdminGetAllUsers` - 获取所有用户
- `POST /AdminDeleteUser` - 删除用户

### 票务相关
- `POST /UserGetTickets` - 获取可用票务
- `POST /UserBuyTicket` - 购买票务
- `POST /UserRefundTicket` - 退票
- `POST /UserGetOwnTickets` - 获取个人票务

### 货币和积分
- `POST /GetCurrency` - 获取货币余额
- `POST /AddCurrency` - 增加货币
- `POST /SubtractCurrency` - 扣除货币

## 谢谢
