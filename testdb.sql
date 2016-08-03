-- ----------------------------
-- Table structure for tb_activity
-- ----------------------------
DROP TABLE IF EXISTS `tb_activity`;
CREATE TABLE `tb_activity` (
  `activity_id` bigint(20) unsigned NOT NULL COMMENT '活动id',
  `activity_name` varchar(100) DEFAULT NULL COMMENT '活动名称',
  `status` tinyint(4) unsigned NOT NULL,
  PRIMARY KEY (`activity_id`),
  KEY `Index_1` (`activity_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='活动表';

-- ----------------------------
-- Records of tb_activity
-- ----------------------------
INSERT INTO `tb_activity` VALUES ('200', 'test mysql sync', '1');
INSERT INTO `tb_activity` VALUES ('819', '测试活动1', '1');
INSERT INTO `tb_activity` VALUES ('824', '测试活动2', '1');
INSERT INTO `tb_activity` VALUES ('832', '测试活动3', '1');
INSERT INTO `tb_activity` VALUES ('834', '测试活动4', '1');
INSERT INTO `tb_activity` VALUES ('887', '测试活动5', '1');
INSERT INTO `tb_activity` VALUES ('888', '测试活动6', '1');
