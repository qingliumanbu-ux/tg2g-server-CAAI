/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:
Version:    1.0
Date:     2020-12-22
Description: 按具体项取消耗
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caai_prt305)
int f_caai_prt305(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	CDbCommand cmd(conn);
	CString  sqlstr("");
	CString  begin_date("");
	CString  end_date("");
	CString  price_terms("");
	CString  sub_backlog_code("");
	CString  mat_type(""), mat_code;
	CDecimal wt = 0;
	CDecimal amt = 0;
	CDecimal m_wt = 0;
	CDecimal m_amt = 0;
	CDecimal price = 0;
	CDecimal use_unit = 0;
	CDecimal m_use_unit = 0;
	int i = 0;

	CModel tcaai04("TCAAI04");
	CModel tcaai06("TCAAI06");
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq1(conn);

	/* ***** 静态变量定义 ***** */
	int 	doFlag = 0;
	int 	fetchRowCount = 0;
	int     RowCount = 0;

	Log::Trace("", "", "1111111111111111111111");
	begin_date = bcls_rec->Tables[0].Rows[0]["BEGIN_DATE"].ToString();
	end_date = bcls_rec->Tables[0].Rows[0]["END_DATE"].ToString();
	price_terms = bcls_rec->Tables[0].Rows[0]["PRICE_TERMS"].ToString();
	sub_backlog_code = bcls_rec->Tables[0].Rows[0]["SUB_BACKLOG_CODE"].ToString();
	mat_code = bcls_rec->Tables[0].Rows[0]["MAT_CODE"].ToString();
	tcaai04.MergeFrom(bcls_rec->Tables[0].Rows[0]);
	tcaai06.MergeFrom(bcls_rec->Tables[0].Rows[0]);

	try
	{
		if (price_terms == "YJ")
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:				// MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = " select  t04.mat_code, sum(case when stats_period = @end_date  then wt else 0 end) wt "
					"   , sum(case when stats_period = @end_date THEN cost_YJ "
					" 	ELSE 0 END) amt "
					" 	, sum(wt) m_wt, sum(cost_YJ) m_amt "
					"   , decode(sum(wt), 0, 0, round(sum(cost_YJ) / sum(wt), 6)) price, "
					"   NVL(use_unit, 0) AS use_unit, NVL(m_use_unit, 0) AS m_use_unit "
					"   from tcaai04 t04"
					" 	left join "
					" 	( "
					" 	select mat_code, decode(sum(case when stats_period = @end_date  then  wt else 0 end), 0, 0, ROUND(sum(case when stats_period = @end_date  then  wt*use_unit else 0 end) / sum(case when stats_period = @end_date  then  wt else 0 end),3)) use_unit "
					" 	, decode(sum(wt), 0, 0, ROUND(sum(wt*use_unit) / sum(wt),3)) m_use_unit "
					" 	from tcaai03 t1 "
					" 	left join tcaac05 t2 on t1.sub_backlog_code = t2.sub_backlog_code and t1.product_code = t2.product_code  "
					" 	where stats_period <= @end_date  and stats_period >=@begin_date ";
				if (sub_backlog_code.Trim() == "B3")
				{
					sqlstr += " and t1.SUB_BACKLOG_CODE in ('B0','B3') ";
				}
				else
				{
					sqlstr += " and t1.SUB_BACKLOG_CODE=@sub_backlog_code ";
				}
				sqlstr += " 	group by mat_code ）t3 on t04.mat_code = t3.mat_code "
					"   WHERE stats_period <= @end_date  and stats_period >=@begin_date ";
				if (sub_backlog_code.Trim() == "B3")
				{
					sqlstr += " and t04.SUB_BACKLOG_CODE in ('B0','B3') ";
				}
				else
				{
					sqlstr += " and t04.SUB_BACKLOG_CODE=@sub_backlog_code ";
				}
				sqlstr += " and t04.mat_code=@mat_code "
					" 	group by  t04.mat_code,use_unit,m_use_unit "
					;
				break;
			}
			Log::Trace("", "", "33333333333333333333");
			cmd_inq1.SetCommandText(sqlstr);
			Log::Trace("", "", "-----sqlstrGJ={0}", sqlstr);
			cmd_inq1.Parameters.Set("begin_date", begin_date.SubstringNE(0, 8));
			cmd_inq1.Parameters.Set("end_date", end_date.SubstringNE(0, 8));
			cmd_inq1.Parameters.Set("sub_backlog_code", sub_backlog_code);
			cmd_inq1.Parameters.Set("mat_code", mat_code);
			cmd_inq1.ExecuteQuery(bcls_ret->Tables[0]);
			cmd_inq1.Close();
			Log::Trace("", "", "END!!!END!!!END!!!END!!!END!!!");
		}
		else if (price_terms == "GJ")
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:				// MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = " select  t04.mat_code, sum(case when prod_date = @end_date  then wt else 0 end) wt "
					"   , sum(case when prod_date = @end_date THEN cost_GJ "
					" 	ELSE 0 END) amt "
					" 	, sum(wt) m_wt, sum(cost_GJ) m_amt "
					"   , decode(sum(wt), 0, 0, round(sum(cost_GJ) / sum(wt), 6)) price, "
					"   NVL(use_unit, 0) AS use_unit, NVL(m_use_unit, 0) AS m_use_unit "
					"   from tcaai06 t04"
					" 	left join "
					" 	( "
					" 	select mat_code, decode(sum(case when prod_date = @end_date  then  wt else 0 end), 0, 0, ROUND(sum(case when prod_date = @end_date  then  wt*use_unit else 0 end) / sum(case when prod_date = @end_date  then  wt else 0 end),3)) use_unit "
					" 	, decode(sum(wt), 0, 0, ROUND(sum(wt*use_unit) / sum(wt),3)) m_use_unit "
					" 	from tcaai05 t1 "
					" 	left join tcaac05 t2 on t1.sub_backlog_code = t2.sub_backlog_code and t1.product_code = t2.product_code  "
					" 	where prod_date <= @end_date  and prod_date >=@begin_date ";
				if (sub_backlog_code.Trim() == "B3")
				{
					sqlstr += " and t1.SUB_BACKLOG_CODE in ('B0','B3') ";
				}
				else
				{
					sqlstr += " and t1.SUB_BACKLOG_CODE=@sub_backlog_code ";
				}
				sqlstr += " 	group by mat_code ）t3 on t04.mat_code = t3.mat_code "
					"   WHERE prod_date <= @end_date  and prod_date >=@begin_date ";
				if (sub_backlog_code.Trim() == "B3")
				{
					sqlstr += " and t04.SUB_BACKLOG_CODE in ('B0','B3') ";
				}
				else
				{
					sqlstr += " and t04.SUB_BACKLOG_CODE=@sub_backlog_code ";
				}
				sqlstr += " and t04.mat_code=@mat_code "
					" 	group by  t04.mat_code,use_unit,m_use_unit "
					;
				break;
			}
			cmd_inq1.SetCommandText(sqlstr);
			Log::Trace("", "", "-----sqlstrGJ={0}", sqlstr);
			cmd_inq1.Parameters.Set("begin_date", begin_date.SubstringNE(0, 8));
			cmd_inq1.Parameters.Set("end_date", end_date.SubstringNE(0, 8));
			cmd_inq1.Parameters.Set("sub_backlog_code", sub_backlog_code);
			cmd_inq1.Parameters.Set("mat_type", mat_type);
			cmd_inq1.Parameters.Set("mat_code", mat_code);
			cmd_inq1.ExecuteQuery(bcls_ret->Tables[0]);
			cmd_inq1.Close();
			Log::Trace("", "", "END!!!END!!!END!!!END!!!END!!!");
		}



	}


	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
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

