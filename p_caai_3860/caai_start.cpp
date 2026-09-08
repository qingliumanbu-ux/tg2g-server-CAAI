/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    1.0
Date:		2012-2-9
Description:
**************************************************/
//框架用头文件
#include "stdafx.h"

//-EP_CODE_VERSION 1
//-EP_SYSTEM_HEAD_BEGIN
//-此节代码请勿更改
/*<remark>=========================================================
/// <summary>
/// 成本导航
/// <para> 1. 前台CAAC13画面的F3(启动)。</para>>
/// <para> 2. 检查参数，参数可为空，空参数视为该参数不参与查询筛选，表示查询全部。对查询参数进行合理性检查。</para>
===========================================================</remark>*/

int f_caai_00(const CString& account_period, const CString& current_status, CDbConnection *conn);
int f_caai_check1(CString stats_period, CString dept_code, CString begin_time, CString end_time, CDbConnection * conn);
int f_caai_check2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_caai_gb(CString stats_period, CString dept_code, CDbConnection * conn);
int f_caai_pcost(CString stats_period, CString dept_code, CDbConnection * conn);
int f_caai_ft(CString stats_period, CString dept_code, CDbConnection * conn);
int f_caai_sj(CString stats_period, CString dept_code, CDbConnection * conn);
int f_caai_cost(CString stats_period, CString dept_code, CString price_terms, CDbConnection * conn);
int f_caai_rate(CString stats_period, CString sub_backlog_code, CString divvy_type, CDbConnection * conn);
int f_caai_ny(CString stats_period, CString dept_code, CDbConnection * conn);
//int f_caai_11(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn) ;
// service入口
BM2F_ENTERACE(caai_start)
//-EP_SYSTEM_HEAD_END

int f_caai_start(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	// 数据库SQL操作字符串
	CString  sqlstr("");

	/* 实体类定义 */
	CModel tcaac13("TCAAC13");

	CDbCommand	cmd_inq(conn);


	int doFlag = 0;

	CString		stats_period = " ";
	CString		job_code = " ";
	CString   begin_time = " ";
	CString   end_time = " ";
	CString   dept_code = "";

	try
	{
		stats_period = bcls_rec->Tables[0].Rows[0]["account_period"].ToString().Trim();
		tcaac13["JOB_CODE"] = bcls_rec->Tables[0].Rows[0]["job_code"].ToString().Trim();
		dept_code = bcls_rec->Tables[0].Rows[0]["dept_code"].ToString().Trim();
		Log::Trace("", "", "stats_period={0},dept_code={1}", stats_period, dept_code);
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句		

			sqlstr = " SELECT S_DATETIME,E_DATETIME\
					 						FROM tcaac06\
																	WHERE STATS_PERIOD = @stats_period\
																							AND		VALID_FLAG	=	'1'\
																													";
			break;
		}
		cmd_inq.SetCommandText(sqlstr);// 设置执行的SQL语句
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			begin_time = cmd_inq.GetString(1);
			end_time = cmd_inq.GetString(2);
		}
		cmd_inq.Close();
		Log::Trace("", "", "begin_time={0},end_time={1}", begin_time, end_time);
		EIClass bcls_rec1;
		EIClass bcls_ret1;

		bcls_rec1.Tables[0].Columns.Add(DT_STRING, "STATS_PERIOD");
		bcls_rec1.Tables[0].Columns.Add(DT_STRING, "BEGIN_TIME");
		bcls_rec1.Tables[0].Columns.Add(DT_STRING, "END_TIME");
		bcls_rec1.Tables[0].Columns.Add(DT_STRING, "DEPT_CODE");
		bcls_rec1.Tables[0].Rows.Add();
		bcls_rec1.Tables[0].Rows[0]["STATS_PERIOD"] = stats_period;
		bcls_rec1.Tables[0].Rows[0]["BEGIN_TIME"] = begin_time;
		bcls_rec1.Tables[0].Rows[0]["END_TIME"] = end_time;
		bcls_rec1.Tables[0].Rows[0]["DEPT_CODE"] = dept_code;

		tcaac13["STATS_PERIOD"] = stats_period;
		tcaac13["DEPT_CODE"] = dept_code;
		EDLog(1, 1, "caai_start.dept_code=[%s]", (const char*)dept_code);

		//开始核算
		if (tcaac13["JOB_CODE"].ToString() == "00")
		{
			tcaac13["JOB_CODE"] = "00";
			doFlag = f_caai_00(stats_period, "1", conn);
			if (doFlag != 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tcaac13["SUCCS_FLAG"] = "1";
			tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
		}
		//生产实绩数据处理
		if (tcaac13["JOB_CODE"].ToString() == "01")
		{
			tcaac13["JOB_CODE"] = "01";
			doFlag = f_caai_check1(stats_period, dept_code, begin_time, end_time, conn);
			if (doFlag != 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tcaac13["SUCCS_FLAG"] = "1";
			tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
		}
		//产副品数据检索及处理
		if (tcaac13["JOB_CODE"].ToString() == "02")
		{
			doFlag = f_caai_check2(&bcls_rec1, &bcls_ret1, conn);
			if (doFlag != 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tcaac13["SUCCS_FLAG"] = "1";
			tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
		}
		//工序产出核算
		if (tcaac13["JOB_CODE"].ToString() == "11")
		{
			doFlag = f_caai_gb(stats_period, dept_code, conn);
			if (doFlag != 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tcaac13["SUCCS_FLAG"] = "1";
			tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
		}

		//工序消耗项核算
		if (tcaac13["JOB_CODE"].ToString() == "12")
		{
			doFlag = f_caai_sj(stats_period, dept_code, conn);
			if (doFlag != 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tcaac13["SUCCS_FLAG"] = "1";
			tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
		}

		if (tcaac13["JOB_CODE"].ToString() == "20")
		{
			//其他方式的分摊方法
// DM8 适配 CHANGE-265:查询。空值搜索 DECODE 改为标准 CASE。
// 改写原因：空值搜索 DECODE 改为标准 CASE,不依赖 NULL 相等匹配的未记载语义；依据 DM 官方文档,DM8 尚未实测。
// 本共用分支面向 DM8,其他 DB_KIND 标签也会执行此 SQL;参数、结果列、条件与排序保持不变。
// 原 SQL（完整保留）：
			// sqlstr = " select distinct sub_backlog_code,divvy_type"
				// " from tcaais2"
				// " where 1=1"
				// " AND dept_code = decode(trim(@dept_code), '', sub_backlog_code, @dept_code)"
				// " and STATS_PERIOD = @stats_period"
				// ;
// DM8 SQL：
			sqlstr = " select distinct sub_backlog_code,divvy_type"
				" from tcaais2"
				" where 1=1"
				" AND dept_code = CASE WHEN trim(@dept_code) IS NULL OR trim(@dept_code) = '' THEN sub_backlog_code ELSE @dept_code END"
				" and STATS_PERIOD = @stats_period"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				doFlag = f_caai_rate(stats_period, cmd_inq.GetString(1), cmd_inq.GetString(2), conn);
				if (doFlag != 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}

			}
			cmd_inq.Close();
			tcaac13["SUCCS_FLAG"] = "1";
			tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
		}

		if (tcaac13["JOB_CODE"].ToString() == "21")
		{
			doFlag = f_caai_ft(stats_period, dept_code, conn);
			if (doFlag != 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tcaac13["SUCCS_FLAG"] = "1";
			tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
		}
		if (tcaac13["JOB_CODE"].ToString() == "22")
		{
			doFlag = f_caai_ny(stats_period, dept_code, conn);
			if (doFlag != 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tcaac13["SUCCS_FLAG"] = "1";
			tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
		}

		if (tcaac13["JOB_CODE"].ToString() == "30")
		{
			doFlag = f_caai_cost(stats_period, dept_code, "PJ", conn);
			if (doFlag != 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}

			tcaac13["SUCCS_FLAG"] = "1";
			tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
		}

		if (tcaac13["JOB_CODE"].ToString() == "31")
		{
			doFlag = f_caai_pcost(stats_period, dept_code, conn);
			if (doFlag != 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tcaac13["SUCCS_FLAG"] = "1";
			tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
		}

		//结束核算
		if (tcaac13["JOB_CODE"].ToString() == "40")
		{
			tcaac13["JOB_CODE"] = "40";
			doFlag = f_caai_00(stats_period, "1", conn);
			if (doFlag != 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tcaac13["SUCCS_FLAG"] = "1";
			tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
		}

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