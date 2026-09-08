/*=========================================================================
//程序名称:     f_caai_01
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:
//修改日期:
//函数说明：需要分摊的数量或金额进行分摊。RULE_TYPE = '2' 分摊重量，RULE_TYPE = '3'分摊金额，RULE_TYPE = '4' 能源特殊
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"
#include "tcaais2.h" 
//程序用头文件

BM2_FUNCTION_EXPORT
int f_caai_ft(CString stats_period, CString dept_code, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;

	CString sqlstr = "";
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CDecimal all_wt, ft_wt, use_all_wt, use_all_amt, sum_wt;
	CString sqlstr_sub = "";
	CString left_no = "";

	CTCAAIS2 tcaais2(conn);

	// 创建电文处理对象
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);
	CDbCommand cmd_inq_sub(conn);

	try
	{

		//产出
		sqlstr = " delete from tcaai02"
			" where 1=1"
			" AND RULE_TYPE not in ('1','4') " //实绩和能源
			" AND DEPT_CODE =@dept_code "
			" AND STATS_PERIOD = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();		


		//更新分摊类型
		sqlstr = " update tcaais2 t1"
			" set DIVVY_TYPE = (select MEASURE_MODE from tcaac07 t2 where t1.mat_code=t2.mat_code and t1.sub_backlog_code = t2.sub_Backlog_code)"
			" where 1=1"
			" and exists (select 1 from tcaac07 t2 where t1.mat_code=t2.mat_code and t1.sub_backlog_code = t2.sub_Backlog_code)"
			" AND DEPT_CODE =@dept_code "
			" AND STATS_PERIOD = @stats_period"
			;
		Log::Trace("", "", "更新分摊类型sqlstr={0}", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//针对需分摊量的，按照分摊系数来核算
		sqlstr = "select *"
			" from tcaais2 "
			" where 1=1"
			" AND DEPT_CODE =@dept_code "
			" AND STATS_PERIOD = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tcaais2);

			if (tcaais2.DIVVY_TYPE == " ")
			{
				tcaais2.DIVVY_TYPE = "1";
			}

			sqlstr_sub = " ";
			if (tcaais2.PROD_DATE.Trim() != "")
			{
				sqlstr_sub = sqlstr_sub + " AND PROD_DATE =@prod_date ";
			}
			if (tcaais2.PROD_SHIFT_GROUP.Trim() != "")
			{
				sqlstr_sub = sqlstr_sub + " AND PROD_SHIFT_GROUP =@prod_shift_group ";
			}
			if (tcaais2.PROD_SHIFT_NO.Trim() != "")
			{
				sqlstr_sub = sqlstr_sub + " AND PROD_SHIFT_NO =@prod_shift_no ";
			}
			if (tcaais2.EQU_NO.Trim() != "")
			{
				sqlstr_sub = sqlstr_sub + " AND EQU_NO =@equ_no ";
			}
			if (tcaais2.PROD_CODE.Trim() != "")
			{
				sqlstr_sub = sqlstr_sub + " AND PROD_CODE =@prod_code ";
			}

			if (tcaais2.DIVVY_TYPE != "7")
			{
				sum_wt = 0;
				//取产量
				sqlstr = " select NVL(SUM(divvy_basic_n),0) from tcaai01 "
					" where 1=1"
					;
				if (tcaais2.PROD_DATE.Trim() != "")
				{
					sqlstr = sqlstr + " AND PROD_DATE =@prod_date ";
				}
				if (tcaais2.PROD_SHIFT_GROUP.Trim() != "")
				{
					sqlstr = sqlstr + " AND PROD_SHIFT_GROUP =@prod_shift_group ";
				}
				if (tcaais2.PROD_SHIFT_NO.Trim() != "")
				{
					sqlstr = sqlstr + " AND PROD_SHIFT_NO =@prod_shift_no ";
				}
				if (tcaais2.EQU_NO.Trim() != "")
				{
					sqlstr = sqlstr + " AND equ_no =@equ_no ";
				}
				if (tcaais2.PROD_CODE.Trim() != "")
				{
					sqlstr = sqlstr + " AND PRODUCT_CODE =@prod_code ";
				}
				sqlstr = sqlstr + " and cost_center=@cost_center "
					" and DIVVY_TYPE = @divvy_type"
					" and dept_code = @dept_code"
					" and stats_period=@stats_period "
					;
				cmd_inq_1.SetCommandText(sqlstr);
				Log::Trace("", "", "产量sqlstr={0}", sqlstr);
				cmd_inq_1.Parameters.Set("stats_period", tcaais2.STATS_PERIOD);
				cmd_inq_1.Parameters.Set("dept_code", tcaais2.DEPT_CODE);
				cmd_inq_1.Parameters.Set("cost_center", tcaais2.SUB_BACKLOG_CODE);
				cmd_inq_1.Parameters.Set("divvy_type", tcaais2.DIVVY_TYPE);
				cmd_inq_1.Parameters.Set("prod_shift_group", tcaais2.PROD_SHIFT_GROUP);
				cmd_inq_1.Parameters.Set("prod_shift_no", tcaais2.PROD_SHIFT_NO);
				cmd_inq_1.Parameters.Set("prod_code", tcaais2.PROD_CODE);
				cmd_inq_1.Parameters.Set("prod_date", tcaais2.PROD_DATE);
				cmd_inq_1.Parameters.Set("equ_no", tcaais2.EQU_NO);
				cmd_inq_1.ExecuteReader();
				if (cmd_inq_1.Read())
				{
					sum_wt = cmd_inq_1.GetDecimal(1);
				}
				cmd_inq_1.Close();

				//Log::Trace("", "", "产量sum_wt={0}", sum_wt);
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					sqlstr = " insert into tcaai02 (REC_CREATE_TIME,dept_code,prod_date,sub_backlog_code,cost_center,stats_period,product_code,equ_no, sg_sign,prod_shift_group,prod_shift_no, mat_thick, mat_width,VFREE1,VFREE2,MAT_CODE,RULE_TYPE,DIVVY_TYPE,WT,AMT,KEY_SEQ)"
						" select @datenow,dept_code,prod_date,sub_backlog_code,cost_center,stats_period,product_CODE,equ_no, sg_sign,prod_shift_group,prod_shift_no, mat_thick, mat_width,VFREE1,VFREE2,@mat_code,@rule_type,DIVVY_TYPE, cast(ROUND(@use_wt*divvy_basic_n/@sum_wt,4) as as decimal(14,4)),cast(ROUND(@use_amt*divvy_basic_n/@sum_wt,2) as decimal(12,2)),'ft'||trim(@prod_shift_group)||trim(@prod_shift_no)"
						" from tcaai01"
						" where 1=1"
						;
					break;
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default: // 通用
					//插入值
					sqlstr = " insert into tcaai02 (REC_CREATE_TIME,dept_code,prod_date,sub_backlog_code,cost_center,stats_period,product_code,equ_no, sg_sign,prod_shift_group,prod_shift_no, mat_thick, mat_width,VFREE1,VFREE2,MAT_CODE,RULE_TYPE,DIVVY_TYPE,WT,AMT,KEY_SEQ)"
						" select @datenow,dept_code,prod_date,sub_backlog_code,cost_center,stats_period,product_CODE,equ_no, sg_sign,prod_shift_group,prod_shift_no, mat_thick, mat_width,VFREE1,VFREE2,@mat_code,@rule_type,DIVVY_TYPE, ROUND(@use_wt*divvy_basic_n/@sum_wt,4),ROUND(@use_amt*divvy_basic_n/@sum_wt,2) ,'ft'||trim(@prod_shift_group)||trim(@prod_shift_no)"
						" from tcaai01"
						" where 1=1"
						;
					break;
				}
				if (tcaais2.PROD_DATE.Trim() != "")
				{
					sqlstr = sqlstr + " AND PROD_DATE =@prod_date ";
				}
				if (tcaais2.PROD_SHIFT_GROUP.Trim() != "")
				{
					sqlstr = sqlstr + " AND PROD_SHIFT_GROUP =@prod_shift_group ";
				}
				if (tcaais2.PROD_SHIFT_NO.Trim() != "")
				{
					sqlstr = sqlstr + " AND PROD_SHIFT_NO =@prod_shift_no ";
				}
				if (tcaais2.EQU_NO.Trim() != "")
				{
					sqlstr = sqlstr + " AND equ_no =@equ_no ";
				}
				if (tcaais2.PROD_CODE.Trim() != "")
				{
					sqlstr = sqlstr + " AND PRODUCT_CODE =@prod_code ";
				}
				sqlstr = sqlstr + " and DIVVY_TYPE = @divvy_type"
					" and dept_code = @dept_code"
					" and cost_center = @cost_center"
					" AND STATS_PERIOD = @stats_period"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				Log::Trace("", "", "sqlstr={0}", sqlstr);
				cmd_inq_1.Parameters.Set("datenow", datenow);
				cmd_inq_1.Parameters.Set("mat_code", tcaais2.MAT_CODE);
				cmd_inq_1.Parameters.Set("rule_type", tcaais2.RULE_TYPE);
				cmd_inq_1.Parameters.Set("use_wt", tcaais2.WT);
				cmd_inq_1.Parameters.Set("sum_wt", sum_wt);
				cmd_inq_1.Parameters.Set("use_amt", tcaais2.AMT);
				cmd_inq_1.Parameters.Set("stats_period", tcaais2.STATS_PERIOD);
				cmd_inq_1.Parameters.Set("dept_code", tcaais2.DEPT_CODE);
				cmd_inq_1.Parameters.Set("cost_center", tcaais2.SUB_BACKLOG_CODE);
				cmd_inq_1.Parameters.Set("divvy_type", tcaais2.DIVVY_TYPE);
				cmd_inq_1.Parameters.Set("prod_code", tcaais2.PROD_CODE);
				cmd_inq_1.Parameters.Set("prod_date", tcaais2.PROD_DATE);
				cmd_inq_1.Parameters.Set("prod_shift_group", tcaais2.PROD_SHIFT_GROUP);
				cmd_inq_1.Parameters.Set("prod_shift_no", tcaais2.PROD_SHIFT_NO);
				cmd_inq_1.Parameters.Set("equ_no", tcaais2.EQU_NO);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();
			}
			else
			{
				//查询系数维护的对应类别是什么
				sqlstr = " select prod_code,sg_sign,mat_thick "
					" FROM TCAAC09 "
					" WHERE SUB_BACKLOG_CODE =@sub_backlog_code "
					" AND MAT_CODE = @mat_code "
					;
				left_no = "";
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("sub_backlog_code", tcaais2.SUB_BACKLOG_CODE);
				cmd_inq_1.Parameters.Set("mat_code", tcaais2.MAT_CODE);
				cmd_inq_1.ExecuteReader();
				if (cmd_inq_1.Read())
				{
					if (cmd_inq_1.GetString(1).Trim() != "")
					{
						left_no = left_no + " and t1.product_code = t9.prod_code";
					}
					if (cmd_inq_1.GetString(2).Trim() != "")
					{
						left_no = left_no + " and t1.sg_sign = t9.sg_sign";
					}
					if (cmd_inq_1.GetDecimal(3) != 0)
					{
						left_no = left_no + " and t1.mat_thick = t9.mat_thick";
					}

				}
				cmd_inq_1.Close();

				//查询单个物料对应所有牌号规格乘上系数后的总量
// DM8 适配 CHANGE-250:查询。空串搜索 DECODE 改为标准 CASE。
// 改写原因：空串搜索 DECODE(x,'',a,b) 改为标准 CASE WHEN x IS NULL OR x='' THEN a ELSE b,与 CHANGE-107 同理,不依赖空串/NULL 匹配的未记载语义；依据 DM 官方文档,DM8 尚未实测。
// 本共用分支面向 DM8,其他 DB_KIND 标签也会执行此 SQL;参数、结果列、条件与排序保持不变。
// 原 SQL（完整保留）：
				// sqlstr = " select sum(wt) from ( "
					// " SELECT  t1.DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, t1.mat_width,SUM(DECODE(CAL_RATE,'',0,CAL_RATE)*divvy_basic_n) wt"
					// " FROM tcaai01 t1 "
					// " left join tcaac09 t9 "
					// " on  t1.cost_center=t9.sub_backlog_code and t9.mat_code=@mat_code " + left_no +
					// " WHERE 1=1 "
					// ;
// DM8 SQL：
				sqlstr = " select sum(wt) from ( "
					" SELECT  t1.DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, t1.mat_width,SUM(CASE WHEN CAL_RATE IS NULL OR CAL_RATE = '' THEN 0 ELSE CAL_RATE END*divvy_basic_n) wt"
					" FROM tcaai01 t1 "
					" left join tcaac09 t9 "
					" on  t1.cost_center=t9.sub_backlog_code and t9.mat_code=@mat_code " + left_no +
					" WHERE 1=1 "
					;
				if (tcaais2.PROD_DATE.Trim() != "")
				{
					sqlstr = sqlstr + " AND PROD_DATE =@prod_date ";
				}
				if (tcaais2.PROD_SHIFT_GROUP.Trim() != "")
				{
					sqlstr = sqlstr + " AND PROD_SHIFT_GROUP =@prod_shift_group ";
				}
				if (tcaais2.PROD_SHIFT_NO.Trim() != "")
				{
					sqlstr = sqlstr + " AND PROD_SHIFT_NO =@prod_shift_no ";
				}
				if (tcaais2.EQU_NO.Trim() != "")
				{
					sqlstr = sqlstr + " AND equ_no =@equ_no ";
				}
				if (tcaais2.PROD_CODE.Trim() != "")
				{
					sqlstr = sqlstr + " AND PRODUCT_CODE =@prod_code ";
				}
				sqlstr = sqlstr + " AND t1.sub_backlog_code = @sub_backlog_code "
					" AND DIVVY_TYPE ='1' " //产量
					" AND stats_period = @stats_period "
					" group by   t1.DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, t1.mat_width "

					" ) ";
				//Log::Trace("", "", "sqlstr={0}", sqlstr);
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("datenow", datenow);
				cmd_inq_1.Parameters.Set("mat_code", tcaais2.MAT_CODE);
				cmd_inq_1.Parameters.Set("sub_backlog_code", tcaais2.SUB_BACKLOG_CODE);
				cmd_inq_1.Parameters.Set("stats_period", stats_period);
				cmd_inq_1.Parameters.Set("dept_code", tcaais2.DEPT_CODE);
				cmd_inq_1.Parameters.Set("cost_center", tcaais2.SUB_BACKLOG_CODE);
				cmd_inq_1.Parameters.Set("divvy_type", tcaais2.DIVVY_TYPE);
				cmd_inq_1.Parameters.Set("prod_code", tcaais2.PROD_CODE);
				cmd_inq_1.Parameters.Set("prod_date", tcaais2.PROD_DATE);
				cmd_inq_1.Parameters.Set("prod_shift_group", tcaais2.PROD_SHIFT_GROUP);
				cmd_inq_1.Parameters.Set("prod_shift_no", tcaais2.PROD_SHIFT_NO);
				cmd_inq_1.Parameters.Set("equ_no", tcaais2.EQU_NO);
				cmd_inq_1.ExecuteReader();
				if (cmd_inq_1.Read())
				{
					sum_wt = cmd_inq_1.GetDecimal(1);
				}
				cmd_inq_1.Close();

				//插入值
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
// DM8 适配 CHANGE-251:写入/查询 TCAAI02。空串搜索 DECODE 改为标准 CASE。
// 改写原因：空串搜索 DECODE(x,'',a,b) 改为标准 CASE WHEN x IS NULL OR x='' THEN a ELSE b,与 CHANGE-107 同理,不依赖空串/NULL 匹配的未记载语义；依据 DM 官方文档,DM8 尚未实测。
// 本共用分支面向 DM8,其他 DB_KIND 标签也会执行此 SQL;参数、结果列、条件与排序保持不变。
// 原 SQL（完整保留）：
					// sqlstr = " insert into tcaai02 (REC_CREATE_TIME,dept_code,prod_date,sub_backlog_code,cost_center,stats_period,product_code,equ_no, sg_sign,prod_shift_group,prod_shift_no, mat_thick, mat_width,VFREE1,VFREE2,MAT_CODE,RULE_TYPE,DIVVY_TYPE,WT,AMT,key_seq)"
						// " select @datenow,dept_code,prod_date,sub_backlog_code,sub_backlog_code,stats_period,product_code,equ_no, sg_sign,prod_shift_group,prod_shift_no, mat_thick, mat_width,VFREE1,VFREE2,MAT_CODE,@rule_type,'7',wt,amt,'ft'||trim(@prod_shift_group)||trim(@prod_shift_no)"
						// " from ("
						// " SELECT t1.product_code,t1.DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, t1.mat_width,t1.VFREE1,t1.VFREE2,@mat_code mat_code,cast(SUM(DECODE(CAL_RATE,'',0,CAL_RATE)*divvy_basic_n)/@sum_wt*@use_wt as decimal(16,4)) wt,cast(SUM(DECODE(CAL_RATE,'',0,CAL_RATE)*divvy_basic_n)/@sum_wt*@use_amt as decimal(12,2) ) amt"
						// " FROM tcaai01 t1 "
						// " left join tcaac09 t9 "
						// " on  t1.cost_center=t9.sub_backlog_code and t9.mat_code=@mat_code " + left_no +
						// " WHERE 1=1 "
						// ;
// DM8 SQL：
					sqlstr = " insert into tcaai02 (REC_CREATE_TIME,dept_code,prod_date,sub_backlog_code,cost_center,stats_period,product_code,equ_no, sg_sign,prod_shift_group,prod_shift_no, mat_thick, mat_width,VFREE1,VFREE2,MAT_CODE,RULE_TYPE,DIVVY_TYPE,WT,AMT,key_seq)"
						" select @datenow,dept_code,prod_date,sub_backlog_code,sub_backlog_code,stats_period,product_code,equ_no, sg_sign,prod_shift_group,prod_shift_no, mat_thick, mat_width,VFREE1,VFREE2,MAT_CODE,@rule_type,'7',wt,amt,'ft'||trim(@prod_shift_group)||trim(@prod_shift_no)"
						" from ("
						" SELECT t1.product_code,t1.DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, t1.mat_width,t1.VFREE1,t1.VFREE2,@mat_code mat_code,cast(SUM(CASE WHEN CAL_RATE IS NULL OR CAL_RATE = '' THEN 0 ELSE CAL_RATE END*divvy_basic_n)/@sum_wt*@use_wt as decimal(16,4)) wt,cast(SUM(CASE WHEN CAL_RATE IS NULL OR CAL_RATE = '' THEN 0 ELSE CAL_RATE END*divvy_basic_n)/@sum_wt*@use_amt as decimal(12,2) ) amt"
						" FROM tcaai01 t1 "
						" left join tcaac09 t9 "
						" on  t1.cost_center=t9.sub_backlog_code and t9.mat_code=@mat_code " + left_no +
						" WHERE 1=1 "
						;
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default: // 通用
					//插入值
// DM8 适配 CHANGE-252:写入/查询 TCAAI02。空串搜索 DECODE 改为标准 CASE。
// 改写原因：空串搜索 DECODE(x,'',a,b) 改为标准 CASE WHEN x IS NULL OR x='' THEN a ELSE b,与 CHANGE-107 同理,不依赖空串/NULL 匹配的未记载语义；依据 DM 官方文档,DM8 尚未实测。
// 本共用分支面向 DM8,其他 DB_KIND 标签也会执行此 SQL;参数、结果列、条件与排序保持不变。
// 原 SQL（完整保留）：
					// sqlstr = " insert into tcaai02 (REC_CREATE_TIME,dept_code,prod_date,sub_backlog_code,cost_center,stats_period,product_code,equ_no, sg_sign,prod_shift_group,prod_shift_no, mat_thick, mat_width,VFREE1,VFREE2,MAT_CODE,RULE_TYPE,DIVVY_TYPE,WT,AMT,key_seq)"
						// " select @datenow,dept_code,prod_date,sub_backlog_code,sub_backlog_code,stats_period,product_code,equ_no, sg_sign,prod_shift_group,prod_shift_no, mat_thick, mat_width,VFREE1,VFREE2,MAT_CODE,@rule_type,'7',wt,amt,'ft'||trim(@prod_shift_group)||trim(@prod_shift_no)"
						// " from ("
						// " SELECT t1.product_code,t1.DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, t1.mat_width,t1.VFREE1,t1.VFREE2,@mat_code mat_code,SUM(DECODE(CAL_RATE,'',0,CAL_RATE)*divvy_basic_n)/@sum_wt*@use_wt as wt,SUM(DECODE(CAL_RATE,'',0,CAL_RATE)*divvy_basic_n)/@sum_wt*@use_amt  as amt"
						// " FROM tcaai01 t1 "
						// " left join tcaac09 t9 "
						// " on  t1.cost_center=t9.sub_backlog_code and t9.mat_code=@mat_code " + left_no +
						// " WHERE 1=1 "
						// ;
// DM8 SQL：
					sqlstr = " insert into tcaai02 (REC_CREATE_TIME,dept_code,prod_date,sub_backlog_code,cost_center,stats_period,product_code,equ_no, sg_sign,prod_shift_group,prod_shift_no, mat_thick, mat_width,VFREE1,VFREE2,MAT_CODE,RULE_TYPE,DIVVY_TYPE,WT,AMT,key_seq)"
						" select @datenow,dept_code,prod_date,sub_backlog_code,sub_backlog_code,stats_period,product_code,equ_no, sg_sign,prod_shift_group,prod_shift_no, mat_thick, mat_width,VFREE1,VFREE2,MAT_CODE,@rule_type,'7',wt,amt,'ft'||trim(@prod_shift_group)||trim(@prod_shift_no)"
						" from ("
						" SELECT t1.product_code,t1.DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, t1.mat_width,t1.VFREE1,t1.VFREE2,@mat_code mat_code,SUM(CASE WHEN CAL_RATE IS NULL OR CAL_RATE = '' THEN 0 ELSE CAL_RATE END*divvy_basic_n)/@sum_wt*@use_wt as wt,SUM(CASE WHEN CAL_RATE IS NULL OR CAL_RATE = '' THEN 0 ELSE CAL_RATE END*divvy_basic_n)/@sum_wt*@use_amt  as amt"
						" FROM tcaai01 t1 "
						" left join tcaac09 t9 "
						" on  t1.cost_center=t9.sub_backlog_code and t9.mat_code=@mat_code " + left_no +
						" WHERE 1=1 "
						;
					break;
				}
				
				if (tcaais2.PROD_DATE.Trim() != "")
				{
					sqlstr = sqlstr + " AND PROD_DATE =@prod_date ";
				}
				if (tcaais2.PROD_SHIFT_GROUP.Trim() != "")
				{
					sqlstr = sqlstr + " AND PROD_SHIFT_GROUP =@prod_shift_group ";
				}
				if (tcaais2.PROD_SHIFT_NO.Trim() != "")
				{
					sqlstr = sqlstr + " AND PROD_SHIFT_NO =@prod_shift_no ";
				}
				if (tcaais2.EQU_NO.Trim() != "")
				{
					sqlstr = sqlstr + " AND equ_no =@equ_no ";
				}
				if (tcaais2.PROD_CODE.Trim() != "")
				{
					sqlstr = sqlstr + " AND PRODUCT_CODE =@prod_code ";
				}
// DM8 适配 CHANGE-253:查询。空串搜索 DECODE 改为标准 CASE。
// 改写原因：空串搜索 DECODE(x,'',a,b) 改为标准 CASE WHEN x IS NULL OR x='' THEN a ELSE b,与 CHANGE-107 同理,不依赖空串/NULL 匹配的未记载语义；依据 DM 官方文档,DM8 尚未实测。
// 本共用分支面向 DM8,其他 DB_KIND 标签也会执行此 SQL;参数、结果列、条件与排序保持不变。
// 原 SQL（完整保留）：
				// sqlstr = sqlstr + " AND t1.sub_backlog_code = @sub_backlog_code "
					// " AND DIVVY_TYPE ='1' " //产量
					// " AND stats_period = @stats_period "
					// " group by   t1.product_code,t1.DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, t1.mat_width,t1.VFREE1,t1.VFREE2 "
					// " HAVING SUM(DECODE(CAL_RATE,'',0,CAL_RATE)*divvy_basic_n)!=0 "
					// ")"
					// ;
// DM8 SQL：
				sqlstr = sqlstr + " AND t1.sub_backlog_code = @sub_backlog_code "
					" AND DIVVY_TYPE ='1' " //产量
					" AND stats_period = @stats_period "
					" group by   t1.product_code,t1.DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, t1.mat_width,t1.VFREE1,t1.VFREE2 "
					" HAVING SUM(CASE WHEN CAL_RATE IS NULL OR CAL_RATE = '' THEN 0 ELSE CAL_RATE END*divvy_basic_n)!=0 "
					")"
					;
				//Log::Trace("", "", "sqlstr={0}", sqlstr);
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("datenow", datenow);
				cmd_inq_1.Parameters.Set("mat_code", tcaais2.MAT_CODE);
				cmd_inq_1.Parameters.Set("rule_type", tcaais2.RULE_TYPE);
				cmd_inq_1.Parameters.Set("use_wt", tcaais2.WT);
				cmd_inq_1.Parameters.Set("use_amt", tcaais2.AMT);
				cmd_inq_1.Parameters.Set("sum_wt", sum_wt);
				cmd_inq_1.Parameters.Set("dept_code", tcaais2.DEPT_CODE);
				cmd_inq_1.Parameters.Set("cost_center", tcaais2.SUB_BACKLOG_CODE);
				cmd_inq_1.Parameters.Set("divvy_type", tcaais2.DIVVY_TYPE);
				cmd_inq_1.Parameters.Set("prod_code", tcaais2.PROD_CODE);
				cmd_inq_1.Parameters.Set("prod_date", tcaais2.PROD_DATE);
				cmd_inq_1.Parameters.Set("prod_shift_group", tcaais2.PROD_SHIFT_GROUP);
				cmd_inq_1.Parameters.Set("prod_shift_no", tcaais2.PROD_SHIFT_NO);
				cmd_inq_1.Parameters.Set("equ_no", tcaais2.EQU_NO);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();

			}

			//误差调整
			sqlstr = "select sum(WT),SUM(AMT)"
				" from tcaai02"
				" where 1=1"
				;
			if (tcaais2.PROD_SHIFT_GROUP.Trim() != "") sqlstr = sqlstr + " AND PROD_SHIFT_GROUP = @prod_shift_group";
			if (tcaais2.PROD_SHIFT_NO.Trim() != "") sqlstr = sqlstr + " AND PROD_SHIFT_NO = @prod_shift_no";
			sqlstr = sqlstr + " AND MAT_CODE = @mat_code"
				//" AND RULE_TYPE =@rule_type "
				" and key_seq = 'ft'||trim(@prod_shift_group)||trim(@prod_shift_no)"
				" and DIVVY_TYPE = @divvy_type"
				" and dept_code = @dept_code"
				" and cost_center = @cost_center"
				" AND STATS_PERIOD = @stats_period"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			//Log::Trace("", "", "sqlstr={0},mat_code={1},stats_period={2},dept_code={3},cost_center={4},rule_type={5},divvy_type={6}", sqlstr, tcaais2.MAT_CODE, tcaais2.STATS_PERIOD, tcaais2.DEPT_CODE, tcaais2.SUB_BACKLOG_CODE, tcaais2.RULE_TYPE, tcaais2.DIVVY_TYPE);
			cmd_inq_1.Parameters.Set("mat_code", tcaais2.MAT_CODE);
			cmd_inq_1.Parameters.Set("stats_period", tcaais2.STATS_PERIOD);
			cmd_inq_1.Parameters.Set("dept_code", tcaais2.DEPT_CODE);
			cmd_inq_1.Parameters.Set("cost_center", tcaais2.SUB_BACKLOG_CODE);
			cmd_inq_1.Parameters.Set("rule_type", tcaais2.RULE_TYPE);
			cmd_inq_1.Parameters.Set("divvy_type", tcaais2.DIVVY_TYPE);
			cmd_inq_1.Parameters.Set("prod_shift_group", tcaais2.PROD_SHIFT_GROUP);
			cmd_inq_1.Parameters.Set("prod_shift_no", tcaais2.PROD_SHIFT_NO);
			cmd_inq_1.ExecuteReader();
			while (cmd_inq_1.Read())
			{
				use_all_wt = cmd_inq_1.GetDecimal(1);
				use_all_amt = cmd_inq_1.GetDecimal(2);
			}
			cmd_inq_1.Close();

			if (tcaais2.RULE_TYPE == "2" && use_all_wt != tcaais2.WT) //分摊的为重量
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					sqlstr = "update tcaai02 "
						" set wt=wt+@use_wt"
						" where 1=1"
						" and wt in (select max(wt) from tcaai02 where 1=1 and mat_code =@mat_code and dept_code = @dept_code and cost_center = @cost_center AND RULE_TYPE =@rule_type  and DIVVY_TYPE = @divvy_type "
						;
					if (tcaais2.PROD_SHIFT_GROUP.Trim() != "") sqlstr = sqlstr + " AND PROD_SHIFT_GROUP = @prod_shift_group";
					if (tcaais2.PROD_SHIFT_NO.Trim() != "") sqlstr = sqlstr + " AND PROD_SHIFT_NO = @prod_shift_no";
					sqlstr = sqlstr + " AND stats_period = @stats_period)"
						" and key_seq = 'ft'||trim(@prod_shift_group)||trim(@prod_shift_no)"
						" AND MAT_CODE = @mat_code"
						" AND RULE_TYPE =@rule_type "
						" and DIVVY_TYPE = @divvy_type"
						;
					if (tcaais2.PROD_SHIFT_GROUP.Trim() != "") sqlstr = sqlstr + " AND PROD_SHIFT_GROUP = @prod_shift_group";
					if (tcaais2.PROD_SHIFT_NO.Trim() != "") sqlstr = sqlstr + " AND PROD_SHIFT_NO = @prod_shift_no";
					sqlstr = sqlstr + " and dept_code = @dept_code"
						" and cost_center = @cost_center"
						" AND STATS_PERIOD = @stats_period"
						" FETCH FIRST 1 ROWS  ONLY"
						;
					break;
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = "update tcaai02 "
						" set wt=wt+@use_wt"
						" where 1=1"
						" and wt in (select max(wt) from tcaai02 where 1=1 and mat_code =@mat_code and dept_code = @dept_code and cost_center = @cost_center AND RULE_TYPE =@rule_type  and DIVVY_TYPE = @divvy_type "
						;
					if (tcaais2.PROD_SHIFT_GROUP.Trim() != "") sqlstr = sqlstr + " AND PROD_SHIFT_GROUP = @prod_shift_group";
					if (tcaais2.PROD_SHIFT_NO.Trim() != "") sqlstr = sqlstr + " AND PROD_SHIFT_NO = @prod_shift_no";
					sqlstr = sqlstr + " AND stats_period = @stats_period)"
						" and key_seq = 'ft'||trim(@prod_shift_group)||trim(@prod_shift_no)"
						" AND MAT_CODE = @mat_code"
						" AND RULE_TYPE =@rule_type "
						" and DIVVY_TYPE = @divvy_type"
						;
					if (tcaais2.PROD_SHIFT_GROUP.Trim() != "") sqlstr = sqlstr + " AND PROD_SHIFT_GROUP = @prod_shift_group";
					if (tcaais2.PROD_SHIFT_NO.Trim() != "") sqlstr = sqlstr + " AND PROD_SHIFT_NO = @prod_shift_no";
					sqlstr = sqlstr + " and dept_code = @dept_code"
						" and cost_center = @cost_center"
						" AND STATS_PERIOD = @stats_period"
						" and rownum=1"
						;
					break;
				}

				cmd_inq_1.SetCommandText(sqlstr);
				//Log::Trace("", "", "sqlstr123={0}", sqlstr);
				cmd_inq_1.Parameters.Set("use_wt", tcaais2.WT - use_all_wt);
				cmd_inq_1.Parameters.Set("mat_code", tcaais2.MAT_CODE);
				cmd_inq_1.Parameters.Set("stats_period", tcaais2.STATS_PERIOD);
				cmd_inq_1.Parameters.Set("dept_code", tcaais2.DEPT_CODE);
				cmd_inq_1.Parameters.Set("cost_center", tcaais2.SUB_BACKLOG_CODE);
				cmd_inq_1.Parameters.Set("rule_type", tcaais2.RULE_TYPE);
				cmd_inq_1.Parameters.Set("divvy_type", tcaais2.DIVVY_TYPE);
				cmd_inq_1.Parameters.Set("prod_shift_group", tcaais2.PROD_SHIFT_GROUP);
				cmd_inq_1.Parameters.Set("prod_shift_no", tcaais2.PROD_SHIFT_NO);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();
			}
			if (tcaais2.RULE_TYPE == "3" && use_all_amt != tcaais2.AMT) //分摊的为金额
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					sqlstr = "update tcaai02 "
						" set amt=amt+@use_amt"
						" where 1=1"
						" and amt in (select max(amt) from tcaai02 where 1=1 and mat_code =@mat_code and dept_code = @dept_code and cost_center = @cost_center AND RULE_TYPE =@rule_type "
						;
					if (tcaais2.PROD_SHIFT_GROUP.Trim() != "") sqlstr = sqlstr + " AND PROD_SHIFT_GROUP = @prod_shift_group";
					if (tcaais2.PROD_SHIFT_NO.Trim() != "") sqlstr = sqlstr + " AND PROD_SHIFT_NO = @prod_shift_no";
					sqlstr = sqlstr + " and DIVVY_TYPE = @divvy_type  AND stats_period = @stats_period)"
						" AND MAT_CODE = @mat_code"
						" AND RULE_TYPE =@rule_type "
						" and key_seq = 'ft'||trim(@prod_shift_group)||trim(@prod_shift_no)"
						;
					if (tcaais2.PROD_SHIFT_GROUP.Trim() != "") sqlstr = sqlstr + " AND PROD_SHIFT_GROUP = @prod_shift_group";
					if (tcaais2.PROD_SHIFT_NO.Trim() != "") sqlstr = sqlstr + " AND PROD_SHIFT_NO = @prod_shift_no";
					sqlstr = sqlstr + " and DIVVY_TYPE = @divvy_type"
						" and dept_code = @dept_code"
						" and cost_center = @cost_center"
						" AND STATS_PERIOD = @stats_period"
						" FETCH FIRST 1 ROWS  ONLY"
						;
					break;
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = "update tcaai02 "
						" set amt=amt+@use_amt"
						" where 1=1"
						" and amt in (select max(amt) from tcaai02 where 1=1 and mat_code =@mat_code and dept_code = @dept_code and cost_center = @cost_center AND RULE_TYPE =@rule_type "
						;
					if (tcaais2.PROD_SHIFT_GROUP.Trim() != "") sqlstr = sqlstr + " AND PROD_SHIFT_GROUP = @prod_shift_group";
					if (tcaais2.PROD_SHIFT_NO.Trim() != "") sqlstr = sqlstr + " AND PROD_SHIFT_NO = @prod_shift_no";
					sqlstr = sqlstr + " and DIVVY_TYPE = @divvy_type  AND stats_period = @stats_period)"
						" AND MAT_CODE = @mat_code"
						" AND RULE_TYPE =@rule_type "
						" and key_seq = 'ft'||trim(@prod_shift_group)||trim(@prod_shift_no)"
						;
					if (tcaais2.PROD_SHIFT_GROUP.Trim() != "") sqlstr = sqlstr + " AND PROD_SHIFT_GROUP = @prod_shift_group";
					if (tcaais2.PROD_SHIFT_NO.Trim() != "") sqlstr = sqlstr + " AND PROD_SHIFT_NO = @prod_shift_no";
					sqlstr = sqlstr + " and DIVVY_TYPE = @divvy_type"
						" and dept_code = @dept_code"
						" and cost_center = @cost_center"
						" AND STATS_PERIOD = @stats_period"
						" and rownum=1"
						;
					break;
				}




				cmd_inq_1.SetCommandText(sqlstr);
				//Log::Trace("", "", "sqlstr123={0}", sqlstr);
				cmd_inq_1.Parameters.Set("use_amt", tcaais2.AMT - use_all_amt);
				cmd_inq_1.Parameters.Set("mat_code", tcaais2.MAT_CODE);
				cmd_inq_1.Parameters.Set("stats_period", tcaais2.STATS_PERIOD);
				cmd_inq_1.Parameters.Set("dept_code", tcaais2.DEPT_CODE);
				cmd_inq_1.Parameters.Set("cost_center", tcaais2.SUB_BACKLOG_CODE);
				cmd_inq_1.Parameters.Set("rule_type", tcaais2.RULE_TYPE);
				cmd_inq_1.Parameters.Set("divvy_type", tcaais2.DIVVY_TYPE);
				cmd_inq_1.Parameters.Set("prod_shift_group", tcaais2.PROD_SHIFT_GROUP);
				cmd_inq_1.Parameters.Set("prod_shift_no", tcaais2.PROD_SHIFT_NO);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();
			}

		}
		cmd_inq.Close();




	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.msg, (const char*)str, sizeof(s.msg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
