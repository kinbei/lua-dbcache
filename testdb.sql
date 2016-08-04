-- ----------------------------
-- Table structure for tb_activity
-- ----------------------------
DROP TABLE IF EXISTS `tb_activity`;
CREATE TABLE `tb_activity` (
  `activity_id` bigint(20) unsigned NOT NULL COMMENT '活动id',
  `activity_name` varchar(100) DEFAULT NULL COMMENT '活动名称',
  PRIMARY KEY (`activity_id`),
  KEY `Index_1` (`activity_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='活动表';

-- ----------------------------
-- Records of tb_activity
-- ----------------------------
INSERT INTO `tb_activity` VALUES ('200', 'test mysql sync');
INSERT INTO `tb_activity` VALUES ('819', 'test activity 1');
INSERT INTO `tb_activity` VALUES ('824', 'test activity 2');
INSERT INTO `tb_activity` VALUES ('832', 'test activity 3');
INSERT INTO `tb_activity` VALUES ('834', 'test activity 4');
INSERT INTO `tb_activity` VALUES ('887', 'test activity 5');
INSERT INTO `tb_activity` VALUES ('888', 'test activity 6');