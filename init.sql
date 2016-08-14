CREATE DATABASE IF NOT EXISTS citta;
GRANT ALL PRIVILEGES ON citta.* to citta@'%' IDENTIFIED BY 'attic';
GRANT ALL PRIVILEGES ON connect.* to citta@'%' IDENTIFIED BY 'attic';
FLUSH PRIVILEGES;
USE citta;

CREATE DATABASE IF NOT EXISTS connect;
GRANT ALL PRIVILEGES ON connect.* to connect@'%' IDENTIFIED BY 'connect';
FLUSH PRIVILEGES;

CREATE TABLE IF NOT EXISTS `names` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `gender` varchar(10) NOT NULL COMMENT '性别',
  `name` varchar(5) NOT NULL COMMENT '名字',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `zen_print_info` (
  `name` varchar(45) NOT NULL COMMENT '管理员姓名',
  `gender` varchar(10) DEFAULT NULL COMMENT '性别',
  `start` int(11) NOT NULL COMMENT '打印起始',
  `end` int(11) NOT NULL COMMENT '打印结束',
  `signtime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '记录写入时间'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `zen_print_images` (
  `name` varchar(45) NOT NULL COMMENT '管理员姓名',
  `array` text NOT NULL COMMENT '打印的内容',
  `signtime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '记录写入时间'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `zen_config` (
  `fahui_name` varchar(45) NOT NULL COMMENT '此次法会名称',
  `last_male_code` int(11) NOT NULL COMMENT '上次皈依男众最大皈依证号，请输入数字',
  `last_female_code` int(11) NOT NULL COMMENT '上次皈依女众皈依最大号，请用数字',
  `date` date DEFAULT NULL COMMENT '记录写入时间',
  PRIMARY KEY (`fahui_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `zen_admin` (
  `name` varchar(45) NOT NULL COMMENT '管理员姓名',
  `ipaddr` varchar(32) NOT NULL COMMENT '管理员的IP地址',
  `signtime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '记录写入时间'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `zen_male` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `receipt` varchar(32) DEFAULT NULL COMMENT '收据号',
  `name` varchar(32) DEFAULT NULL COMMENT '姓名',
  `phone_num` varchar(20) DEFAULT NULL COMMENT '手机号',
  `gender` varchar(10) DEFAULT NULL COMMENT '性别',
  `fname` varchar(32) DEFAULT NULL COMMENT '法名',
  `race` varchar(20) DEFAULT NULL COMMENT '民族',
  `birthday` varchar(32) DEFAULT NULL COMMENT '生日',
  `degree` varchar(32) DEFAULT NULL COMMENT '学历',
  `province` varchar(64) DEFAULT NULL COMMENT '现居省',
  `city` varchar(64) DEFAULT NULL COMMENT '先居市',
  `district` varchar(64) DEFAULT NULL COMMENT '现居区县',
  `address` varchar(64) DEFAULT NULL COMMENT '详细街道地址',
  `code` varchar(64) DEFAULT NULL COMMENT '皈依证号',
  `job` varchar(32) DEFAULT NULL COMMENT '工作',
  `hobby` varchar(64) DEFAULT NULL COMMENT '爱好',
  `personnel_id` varchar(32) DEFAULT NULL COMMENT '身份证号码',
  `health` varchar(32) DEFAULT NULL COMMENT '健康状况',
  `telephone_num` varchar(20) DEFAULT NULL COMMENT '电话号码',
  `edit_time` varchar(32) DEFAULT NULL COMMENT '编辑时间',
  `workplace` varchar(64) DEFAULT NULL COMMENT '工作单位',
  `postcode` varchar(64) DEFAULT NULL COMMENT '邮编号码',
  `graduate_time` varchar(32) DEFAULT NULL COMMENT '毕业时间',
  `graduate_school` varchar(64) DEFAULT NULL COMMENT '毕业学校',
  `first_job_entry_time` varchar(64) DEFAULT NULL COMMENT '第一份工作入职时间',
  `first_job_workplace` varchar(64) DEFAULT NULL COMMENT '第一份工作入职单位',
  `second_job_entry_time` varchar(32) DEFAULT NULL COMMENT '第二份工作入职时间',
  `second_job_workplace` varchar(64) DEFAULT NULL COMMENT '第二份工作入职单位',
  `retirement_date` varchar(32) DEFAULT NULL COMMENT '退休时间',
  `retirement_workplace` varchar(64) DEFAULT NULL COMMENT '退休工作单位',
  `year2start_learning_buddhism` varchar(32) DEFAULT NULL COMMENT '学佛起始时间',
  `years_of_learning_buddhism` varchar(10) DEFAULT NULL COMMENT '学佛时间（年）',
  `deep_understanding_of_dharma` varchar(64) DEFAULT NULL COMMENT '是否深刻理解佛法，层次',
  `reason2learning_dharma` varchar(128) DEFAULT NULL COMMENT '学佛因缘',
  `nums_of_buddhism_book` varchar(10) DEFAULT NULL COMMENT '读过经典部数',
  `easy2learn_buddhism_book` varchar(128) DEFAULT NULL COMMENT '认为易学的经典',
  `hard2read` varchar(128) DEFAULT NULL COMMENT '认为难读懂的经典',
  `maxim` varchar(128) DEFAULT NULL COMMENT '格言',
  `buddhist_disciples_of_family` varchar(128) DEFAULT NULL COMMENT '家庭三宝地址',
  `editor` varchar(128) DEFAULT NULL COMMENT '编辑人姓名',
  `others` varchar(128) DEFAULT NULL COMMENT '修改人姓名',
  `learn_dharma_kinds` varchar(64) DEFAULT NULL COMMENT '学佛小组种类',
  `learn_dharma_address` varchar(128) DEFAULT NULL COMMENT '学佛小组地址',
  `if_apply_learn_place` varchar(10) DEFAULT '否' COMMENT '是否提供学佛场地',
  `notes` varchar(128) DEFAULT NULL COMMENT '其他信息',
  `mark` tinyint(1) NOT NULL DEFAULT '0' COMMENT '标记',
  `ipaddress` varchar(32) DEFAULT NULL COMMENT 'IP 地址',
  `comes` tinyint(4) NOT NULL DEFAULT '0' COMMENT '来自手机或者笔记本录入, 0: 笔记本, 1: 手机',
  `mod_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '记录写入时间',
  `print_mark` tinyint(4) DEFAULT '0' COMMENT '打印比较，默认值为0,打印成功为1',
  `data_ready_mark` tinyint(4) DEFAULT '0' COMMENT '数据准备情况，当地址信息不为空时，认为数据准备完成。',
  `fahui_name` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `code` (`code`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `zen_female` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `receipt` varchar(32) DEFAULT NULL COMMENT '收据号',
  `name` varchar(32) DEFAULT NULL COMMENT '姓名',
  `phone_num` varchar(20) DEFAULT NULL COMMENT '手机号',
  `gender` varchar(10) DEFAULT NULL COMMENT '性别',
  `fname` varchar(32) DEFAULT NULL COMMENT '法名',
  `race` varchar(20) DEFAULT NULL COMMENT '民族',
  `birthday` varchar(32) DEFAULT NULL COMMENT '生日',
  `degree` varchar(32) DEFAULT NULL COMMENT '学历',
  `province` varchar(64) DEFAULT NULL COMMENT '现居省',
  `city` varchar(64) DEFAULT NULL COMMENT '先居市',
  `district` varchar(64) DEFAULT NULL COMMENT '现居区县',
  `address` varchar(64) DEFAULT NULL COMMENT '详细街道地址',
  `code` varchar(64) DEFAULT NULL COMMENT '皈依证号',
  `job` varchar(32) DEFAULT NULL COMMENT '工作',
  `hobby` varchar(64) DEFAULT NULL COMMENT '爱好',
  `personnel_id` varchar(32) DEFAULT NULL COMMENT '身份证号码',
  `health` varchar(32) DEFAULT NULL COMMENT '健康状况',
  `telephone_num` varchar(20) DEFAULT NULL COMMENT '电话号码',
  `edit_time` varchar(32) DEFAULT NULL COMMENT '编辑时间',
  `workplace` varchar(64) DEFAULT NULL COMMENT '工作单位',
  `postcode` varchar(64) DEFAULT NULL COMMENT '邮编号码',
  `graduate_time` varchar(32) DEFAULT NULL COMMENT '毕业时间',
  `graduate_school` varchar(64) DEFAULT NULL COMMENT '毕业学校',
  `first_job_entry_time` varchar(64) DEFAULT NULL COMMENT '第一份工作入职时间',
  `first_job_workplace` varchar(64) DEFAULT NULL COMMENT '第一份工作入职单位',
  `second_job_entry_time` varchar(32) DEFAULT NULL COMMENT '第二份工作入职时间',
  `second_job_workplace` varchar(64) DEFAULT NULL COMMENT '第二份工作入职单位',
  `retirement_date` varchar(32) DEFAULT NULL COMMENT '退休时间',
  `retirement_workplace` varchar(64) DEFAULT NULL COMMENT '退休工作单位',
  `year2start_learning_buddhism` varchar(32) DEFAULT NULL COMMENT '学佛起始时间',
  `years_of_learning_buddhism` varchar(10) DEFAULT NULL COMMENT '学佛时间（年）',
  `deep_understanding_of_dharma` varchar(64) DEFAULT NULL COMMENT '是否深刻理解佛法，层次',
  `reason2learning_dharma` varchar(128) DEFAULT NULL COMMENT '学佛因缘',
  `nums_of_buddhism_book` varchar(10) DEFAULT NULL COMMENT '读过经典部数',
  `easy2learn_buddhism_book` varchar(128) DEFAULT NULL COMMENT '认为易学的经典',
  `hard2read` varchar(128) DEFAULT NULL COMMENT '认为难读懂的经典',
  `maxim` varchar(128) DEFAULT NULL COMMENT '格言',
  `buddhist_disciples_of_family` varchar(128) DEFAULT NULL COMMENT '家庭三宝地址',
  `editor` varchar(128) DEFAULT NULL COMMENT '编辑人姓名',
  `others` varchar(128) DEFAULT NULL COMMENT '修改人姓名',
  `learn_dharma_kinds` varchar(64) DEFAULT NULL COMMENT '学佛小组种类',
  `learn_dharma_address` varchar(128) DEFAULT NULL COMMENT '学佛小组地址',
  `if_apply_learn_place` varchar(10) DEFAULT '否' COMMENT '是否提供学佛场地',
  `notes` varchar(128) DEFAULT NULL COMMENT '其他信息',
  `mark` tinyint(1) NOT NULL DEFAULT '0' COMMENT '标记',
  `ipaddress` varchar(32) DEFAULT NULL COMMENT 'IP 地址',
  `comes` tinyint(4) NOT NULL DEFAULT '0' COMMENT '来自手机或者笔记本录入, 0: 笔记本, 1: 手机',
  `mod_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '记录写入时间',
  `print_mark` tinyint(4) DEFAULT '0' COMMENT '打印比较，默认值为0,打印成功为1',
  `data_ready_mark` tinyint(4) DEFAULT '0' COMMENT '数据准备情况，当地址信息不为空时，认为数据准备完成。',
  `fahui_name` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `code` (`code`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;
