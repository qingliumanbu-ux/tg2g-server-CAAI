/*=========================================================================
//程序名称:     f_caai_01
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:     
//修改日期:  
//函数说明：对分摊的基数服务量维护。
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件

BM2_FUNCTION_EXPORT
int f_caai_rate(CString stats_period, CString sub_backlog_code, CString divvy_type ,CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;	

	CString sqlstr = "";
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");	
	CDecimal all_wt ,ft_wt,use_all_wt;

	// 创建电文处理对象
	CDbCommand cmd_inq(conn);

	try
	{
		if (divvy_type == "1")
		{
			//1、产量法
// DM8 适配 CHANGE-255:删除 TCAAI01。空值搜索 DECODE 改为标准 CASE。
// 改写原因：空值搜索 DECODE 改为标准 CASE,不依赖 NULL 相等匹配的未记载语义；依据 DM 官方文档,DM8 尚未实测。
// 本共用分支面向 DM8,其他 DB_KIND 标签也会执行此 SQL;参数、结果列、条件与排序保持不变。
// 原 SQL（完整保留）：
			// sqlstr = " delete from tcaai01"
				// " where 1=1"
				// " AND sub_backlog_code = decode(trim(@sub_backlog_code), '', sub_backlog_code, @sub_backlog_code)"
				// " and divvy_type = '1'"
				// " and STATS_PERIOD = @stats_period"
				// ;
// DM8 SQL：
			sqlstr = " delete from tcaai01"
				" where 1=1"
				" AND sub_backlog_code = CASE WHEN trim(@sub_backlog_code) IS NULL OR trim(@sub_backlog_code) = '' THEN sub_backlog_code ELSE @sub_backlog_code END"
				" and divvy_type = '1'"
				" and STATS_PERIOD = @stats_period"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.Parameters.Set("sub_backlog_code", sub_backlog_code);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

// DM8 适配 CHANGE-256:写入/查询 TCAAI01。空值搜索 DECODE 改为标准 CASE。
// 改写原因：空值搜索 DECODE 改为标准 CASE,不依赖 NULL 相等匹配的未记载语义；依据 DM 官方文档,DM8 尚未实测。
// 本共用分支面向 DM8,其他 DB_KIND 标签也会执行此 SQL;参数、结果列、条件与排序保持不变。
// 原 SQL（完整保留）：
			// sqlstr = "insert into tcaai01(REC_CREATE_TIME,dept_code, stats_period, cost_center, prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, unit_code, sg_sign, mat_thick, mat_width, product_code, product_code_cname, divvy_type, divvy_basic_n )"
				// " SELECT @datenow,  DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name,'1',SUM(WT)"
				// " FROM TCAAIS1"
				// " WHERE 1=1"
				// " AND sub_backlog_code = decode(trim(@sub_backlog_code),'',sub_backlog_code,@sub_backlog_code)"
				// " AND PRO_FLAG ='O' " //产出工序
				// " AND stats_period = @stats_period"
				// " group by   DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name "
				// " HAVING SUM(WT)!=0"
				// ;
// DM8 SQL：
			sqlstr = "insert into tcaai01(REC_CREATE_TIME,dept_code, stats_period, cost_center, prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, unit_code, sg_sign, mat_thick, mat_width, product_code, product_code_cname, divvy_type, divvy_basic_n )"
				" SELECT @datenow,  DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name,'1',SUM(WT)"
				" FROM TCAAIS1"
				" WHERE 1=1"
				" AND sub_backlog_code = CASE WHEN trim(@sub_backlog_code) IS NULL OR trim(@sub_backlog_code) = '' THEN sub_backlog_code ELSE @sub_backlog_code END"
				" AND PRO_FLAG ='O' " //产出工序
				" AND stats_period = @stats_period"
				" group by   DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name "
				" HAVING SUM(WT)!=0"
				;
			cmd_inq.SetCommandText(sqlstr);
			//Log::Trace("", "", "sqlstr={0}", sqlstr);
			cmd_inq.Parameters.Set("datenow", datenow);
			cmd_inq.Parameters.Set("sub_backlog_code", sub_backlog_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();
		}


		if (divvy_type == "8")
		{
			//8、表面积分摊法
// DM8 适配 CHANGE-257:删除 TCAAI01。空值搜索 DECODE 改为标准 CASE。
// 改写原因：空值搜索 DECODE 改为标准 CASE,不依赖 NULL 相等匹配的未记载语义；依据 DM 官方文档,DM8 尚未实测。
// 本共用分支面向 DM8,其他 DB_KIND 标签也会执行此 SQL;参数、结果列、条件与排序保持不变。
// 原 SQL（完整保留）：
			// sqlstr = " delete from tcaai01"
				// " where 1=1"
				// " AND sub_backlog_code = decode(trim(@sub_backlog_code), '', sub_backlog_code, @sub_backlog_code)"
				// " and divvy_type = '8'"
				// " and STATS_PERIOD = @stats_period"
				// ;
// DM8 SQL：
			sqlstr = " delete from tcaai01"
				" where 1=1"
				" AND sub_backlog_code = CASE WHEN trim(@sub_backlog_code) IS NULL OR trim(@sub_backlog_code) = '' THEN sub_backlog_code ELSE @sub_backlog_code END"
				" and divvy_type = '8'"
				" and STATS_PERIOD = @stats_period"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.Parameters.Set("sub_backlog_code", sub_backlog_code);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

// DM8 适配 CHANGE-258:写入/查询 TCAAI01。空值搜索 DECODE 改为标准 CASE。
// 改写原因：空值搜索 DECODE 改为标准 CASE,不依赖 NULL 相等匹配的未记载语义；依据 DM 官方文档,DM8 尚未实测。
// 本共用分支面向 DM8,其他 DB_KIND 标签也会执行此 SQL;参数、结果列、条件与排序保持不变。
// 原 SQL（完整保留）：
			// sqlstr = "insert into tcaai01(REC_CREATE_TIME,dept_code, stats_period, cost_center, prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, unit_code, sg_sign, mat_thick, mat_width, product_code, product_code_cname, divvy_type, divvy_basic_n )"
				// " SELECT @datenow,  DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_time, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name,'8',SUM(WT)"
				// " from ( select DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_time, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name,2*mat_thick*mat_width+2*(mat_thick+mat_width)*mat_len WT"
				// " FROM TCAAIA1"
				// " WHERE 1=1"
				// " AND sub_backlog_code = decode(trim(@sub_backlog_code),'',sub_backlog_code,@sub_backlog_code)"
				// " AND PRO_FLAG ='O' " //产出工序
				// " AND stats_period = @stats_period"
				// " )"
				// " group by   DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_time, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name "
				// " HAVING SUM(WT)!=0"
				// ;
// DM8 SQL：
			sqlstr = "insert into tcaai01(REC_CREATE_TIME,dept_code, stats_period, cost_center, prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, unit_code, sg_sign, mat_thick, mat_width, product_code, product_code_cname, divvy_type, divvy_basic_n )"
				" SELECT @datenow,  DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_time, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name,'8',SUM(WT)"
				" from ( select DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_time, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name,2*mat_thick*mat_width+2*(mat_thick+mat_width)*mat_len WT"
				" FROM TCAAIA1"
				" WHERE 1=1"
				" AND sub_backlog_code = CASE WHEN trim(@sub_backlog_code) IS NULL OR trim(@sub_backlog_code) = '' THEN sub_backlog_code ELSE @sub_backlog_code END"
				" AND PRO_FLAG ='O' " //产出工序
				" AND stats_period = @stats_period"
				" )"
				" group by   DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_time, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name "
				" HAVING SUM(WT)!=0"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("datenow", datenow);
			cmd_inq.Parameters.Set("sub_backlog_code", sub_backlog_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();
		}

		if (divvy_type =="9")
		{
			//9、长度分摊法
// DM8 适配 CHANGE-259:删除 TCAAI01。空值搜索 DECODE 改为标准 CASE。
// 改写原因：空值搜索 DECODE 改为标准 CASE,不依赖 NULL 相等匹配的未记载语义；依据 DM 官方文档,DM8 尚未实测。
// 本共用分支面向 DM8,其他 DB_KIND 标签也会执行此 SQL;参数、结果列、条件与排序保持不变。
// 原 SQL（完整保留）：
			// sqlstr = " delete from tcaai01"
				// " where 1=1"
				// " AND sub_backlog_code = decode(trim(@sub_backlog_code), '', sub_backlog_code, @sub_backlog_code)"
				// " and divvy_type = '9'"
				// " and STATS_PERIOD = @stats_period"
// 
				// ;
// DM8 SQL：
			sqlstr = " delete from tcaai01"
				" where 1=1"
				" AND sub_backlog_code = CASE WHEN trim(@sub_backlog_code) IS NULL OR trim(@sub_backlog_code) = '' THEN sub_backlog_code ELSE @sub_backlog_code END"
				" and divvy_type = '9'"
				" and STATS_PERIOD = @stats_period"

				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.Parameters.Set("sub_backlog_code", sub_backlog_code);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

// DM8 适配 CHANGE-260:写入/查询 TCAAI01。空值搜索 DECODE 改为标准 CASE。
// 改写原因：空值搜索 DECODE 改为标准 CASE,不依赖 NULL 相等匹配的未记载语义；依据 DM 官方文档,DM8 尚未实测。
// 本共用分支面向 DM8,其他 DB_KIND 标签也会执行此 SQL;参数、结果列、条件与排序保持不变。
// 原 SQL（完整保留）：
			// sqlstr = "insert into tcaai01(REC_CREATE_TIME,dept_code, stats_period, cost_center, prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, unit_code, sg_sign, mat_thick, mat_width, product_code, product_code_cname, divvy_type, divvy_basic_n )"
				// " SELECT @datenow,  DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_time, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name,'9',SUM(WT)"
				// " from ( select DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_time, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name,mat_len WT"
				// " FROM TCAAIA1"
				// " WHERE 1=1"
				// " AND sub_backlog_code = decode(trim(@sub_backlog_code),'',sub_backlog_code,@sub_backlog_code)"
				// " AND PRO_FLAG ='O' " //产出工序
				// " AND stats_period = @stats_period"
				// " )"
				// " group by   DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_time, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name "
				// " HAVING SUM(WT)!=0"
				// ;
// DM8 SQL：
			sqlstr = "insert into tcaai01(REC_CREATE_TIME,dept_code, stats_period, cost_center, prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, unit_code, sg_sign, mat_thick, mat_width, product_code, product_code_cname, divvy_type, divvy_basic_n )"
				" SELECT @datenow,  DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_time, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name,'9',SUM(WT)"
				" from ( select DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_time, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name,mat_len WT"
				" FROM TCAAIA1"
				" WHERE 1=1"
				" AND sub_backlog_code = CASE WHEN trim(@sub_backlog_code) IS NULL OR trim(@sub_backlog_code) = '' THEN sub_backlog_code ELSE @sub_backlog_code END"
				" AND PRO_FLAG ='O' " //产出工序
				" AND stats_period = @stats_period"
				" )"
				" group by   DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_time, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,MAT_CODE,mat_name "
				" HAVING SUM(WT)!=0"
				;
			cmd_inq.SetCommandText(sqlstr);
			//Log::Trace("", "", "sqlstr123={0}", sqlstr);
			cmd_inq.Parameters.Set("datenow", datenow);
			cmd_inq.Parameters.Set("sub_backlog_code", sub_backlog_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();
		}

	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.msg, (const char*)str, sizeof(s.msg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
