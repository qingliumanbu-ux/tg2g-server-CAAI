/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:
Version:    1.0
Date:     2020-12-22
Description: 炼钢产量查询
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caai_prt20201)
int f_caai_prt20201(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	CDbCommand cmd(conn);
	CString  sqlstr("");
	CString  begin_date("");
	CString  end_date("");
	CString  price_terms("");
	CString  sub_backlog_code("");
	CString  equ_no(""), mat_type;

	int i = 0;

	CModel tcaai03("TCAAI03");
	CModel tcaai05("TCAAI05");
	CDbCommand cmd_inq(conn);

	/* ***** 静态变量定义 ***** */
	int 	doFlag = 0;
	int 	fetchRowCount = 0;
	int     RowCount = 0;


	begin_date = bcls_rec->Tables[0].Rows[0]["BEGIN_DATE"].ToString();
	end_date = bcls_rec->Tables[0].Rows[0]["END_DATE"].ToString();
	price_terms = bcls_rec->Tables[0].Rows[0]["PRICE_TERMS"].ToString();
	//sub_backlog_code = bcls_rec->Tables[0].Rows[0]["SUB_BACKLOG_CODE"].ToString();
	/*mat_type = bcls_rec->Tables[0].Rows[0]["MAT_TYPE"].ToString();
	equ_no = bcls_rec->Tables[0].Rows[0]["EQU_NO"].ToString();*/
	tcaai03.MergeFrom(bcls_rec->Tables[0].Rows[0]);
	tcaai05.MergeFrom(bcls_rec->Tables[0].Rows[0]);
	Log::Trace("", "", "BEGIN!!!BEGIN!!!BEGIN!!!BEGIN!!!BEGIN!!!");
	try
	{
		if (price_terms.Trim() == "YJ")
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = " select SUM( case when stats_period>=@begin_date  and stats_period<=@end_date  and sub_backlog_code = 'C' THEN wt ELSE 0 END) out_wt "
					" 	, SUM(case when  sub_backlog_code = 'C' THEN wt ELSE 0 END)  m_out_WT "
					/*" , SUM(case when stats_period >= @begin_date  and stats_period <= @end_date and sub_backlog_code = 'R' THEN wt ELSE 0 END) out_wt_RH "
					" 	, SUM(case when  sub_backlog_code = 'R' THEN wt ELSE 0 END)  m_out_WT_RH "*/
					" 	FROM TCAAI03 "
					" 	WHERE STATS_PERIOD <= @end_date "
					" 	AND  STATS_PERIOD >= (select max(S_DATETIME) from tcaac15 where S_DATETIME <= @begin_date) "
					//" 	and SUB_BACKLOG_CODE=@sub_backlog_code "
					;
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", "", "-----sqlstrGJ={0}", sqlstr);
			cmd_inq.Parameters.Set("begin_date", begin_date.SubstringNE(0, 8));
			cmd_inq.Parameters.Set("end_date", end_date.SubstringNE(0, 8));
			//cmd_inq.Parameters.Set("sub_backlog_code", sub_backlog_code);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
			cmd_inq.Close();
			Log::Trace("", "", "END!!!END!!!END!!!END!!!END!!!");
		}
		else if (price_terms.Trim() == "GJ")
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = " select SUM( case when prod_date>=@begin_date  and prod_date<=@end_date  and sub_backlog_code = 'C' THEN wt ELSE 0 END) out_wt "
					" , SUM(case when  sub_backlog_code = 'C' THEN wt ELSE 0 END)  m_out_WT "
				/*	" , SUM(case when account_period = substr(@end_date, 0, 6) and sub_backlog_code = 'R' THEN wt ELSE 0 END) out_wt_RH "
					" , SUM(case when  sub_backlog_code = 'R' THEN wt ELSE 0 END)  m_out_WT_RH "*/
					" FROM TCAAI05 "
					" WHERE substr(account_period, 0, 4) = substr(@end_date, 0, 4) "
					//" and SUB_BACKLOG_CODE=@sub_backlog_code "
					;
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", "", "-----sqlstrGJ={0}", sqlstr);
			cmd_inq.Parameters.Set("begin_date", begin_date.SubstringNE(0, 8));
			cmd_inq.Parameters.Set("end_date", end_date.SubstringNE(0, 8));
			//cmd_inq.Parameters.Set("sub_backlog_code", sub_backlog_code);
			//cmd_inq.Parameters.Set("sub_backlog_code", tcaai05["SUB_BACKLOG_CODE"].ToString());
			/*cmd_inq.Parameters.Set("mat_type", tcaai05["MAT_TYPE"].ToString());
			cmd_inq.Parameters.Set("equ_no", tcaai05["EQU_NO"].ToString());*/
			cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
			cmd_inq.Close();
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

