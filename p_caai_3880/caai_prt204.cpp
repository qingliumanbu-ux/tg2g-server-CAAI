/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:
Version:    1.0
Date:     2020-12-22
Description: 炼钢按分类查消耗
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caai_prt204)
int f_caai_prt204(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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
	Log::Trace("", "", "-----price_terms={0}", price_terms);
	mat_type = bcls_rec->Tables[0].Rows[0]["MAT_TYPE"].ToString();
	Log::Trace("", "", "-----mat_type={0}", mat_type);
	//sub_backlog_code = bcls_rec->Tables[0].Rows[0]["SUB_BACKLOG_CODE"].ToString();
	tcaai04.MergeFrom(bcls_rec->Tables[0].Rows[0]);
	tcaai06.MergeFrom(bcls_rec->Tables[0].Rows[0]);

	try
	{
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = " select t.mat_code, t11.mat_name,max(t.SCHE_NO) as SCHE_NO,0 wt,0 amt,0 m_wt,0 m_amt,0 use_unit,0 m_use_unit,0 price from tcaac03 t "
				" left join tcaac11 t11 on t.mat_code = t11.mat_code "
				" where 1=1 "
				" and back_code_2 = @mat_type "
				" AND SUB_BACKLOG_CODE in('E', 'L', 'R', 'C')"
				" group by   t.mat_code, t11.mat_name "
				" ORDER BY SCHE_NO "
				;
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", "", "-----sqlstr={0}", sqlstr);
		//cmd_inq.Parameters.Set("sub_backlog_code", sub_backlog_code);
		cmd_inq.Parameters.Set("mat_type", mat_type);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();
		Log::Trace("", "", "22222222222222222222222");
		for (int i = 0; i < bcls_ret->Tables[0].Rows.get_Count(); i++)
		{
			mat_code = bcls_ret->Tables[0].Rows[i]["MAT_CODE"].ToString().Trim();
			Log::Trace("", "", "-----mat_code={0}", mat_code);
			if (price_terms == "YJ")
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:				// MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = " select  t04.mat_code, sum(case when stats_period >= @begin_date and stats_period <= @end_date  then wt else 0 end) wt "
						"   , sum(case when stats_period >= @begin_date  and stats_period <= @end_date THEN cost_YJ "
						" 	ELSE 0 END) amt "
						" 	, sum(wt) m_wt, sum(cost_YJ) m_amt "
						"   ,decode(sum(wt), 0, 0, round(sum(cost_YJ) / sum(wt),6)) price, "
						"   NVL(use_unit, 0) AS use_unit, NVL(m_use_unit, 0) AS m_use_unit "
						"   from tcaai04 t04"
						" 	left join "
						" 	( "
						" 	select mat_code, decode(sum(case when stats_period >= @begin_date and stats_period <= @end_date  then  wt else 0 end), 0, 0, ROUND(sum(case when stats_period >= @begin_date and stats_period <= @end_date  then  wt*use_unit else 0 end) / sum(case when stats_period >= @begin_date and stats_period <= @end_date  then  wt else 0 end),3)) use_unit "
						" 	, decode(sum(wt), 0, 0, ROUND(sum(wt*use_unit) / sum(wt),3)) m_use_unit "
						" 	from tcaai03 t1 "
						" 	left join tcaac05 t2 on t1.sub_backlog_code = t2.sub_backlog_code and t1.product_code = t2.product_code "  //
						//"   left join tcaac05c t2 on t1.sg_sign = t2.sg_sign  and substr(t1.product_code,0,9) = t2.product_code "  //取BOM值
						" 	and  mat_code in(select mat_code from tcaac03 where back_code_2 = @mat_type) "
						" 	where stats_period <= (select max(E_DATETIME) from tcaac15 where S_DATETIME <= @end_date)  and stats_period >= (select max(S_DATETIME) from tcaac15 where S_DATETIME <= @begin_date) "
						"   and t1.sub_backlog_code in('E', 'L', 'R', 'C') "
						" 	group by mat_code ）t3 on t04.mat_code = t3.mat_code "
						" 	where t04.mat_code in(select mat_code from tcaac03 where back_code_2 = @mat_type) "
						"   and stats_period <= (select max(E_DATETIME) from tcaac15 where S_DATETIME <= @end_date) and stats_period >= (select max(S_DATETIME) from tcaac15 where S_DATETIME <= @begin_date) "
						" 	and sub_backlog_code in('E', 'L', 'R', 'C') "
						"   and t04.mat_code=@mat_code "
						" 	group by  t04.mat_code,use_unit,m_use_unit "
						;
					break;
				}
				Log::Trace("", "", "33333333333333333333");
				cmd_inq1.SetCommandText(sqlstr);
				cmd_inq1.Parameters.Set("begin_date", begin_date.SubstringNE(0, 8));
				cmd_inq1.Parameters.Set("end_date", end_date.SubstringNE(0, 8));
				//cmd_inq1.Parameters.Set("sub_backlog_code", sub_backlog_code);
				cmd_inq1.Parameters.Set("mat_type", mat_type);
				cmd_inq1.Parameters.Set("mat_code", mat_code);
				cmd_inq1.ExecuteReader();
				if (cmd_inq1.Read())
				{
					wt = cmd_inq1.GetDecimal(2);
					Log::Trace("", "", "-----wt={0}", wt);
					amt = cmd_inq1.GetDecimal(3);
					m_wt = cmd_inq1.GetDecimal(4);
					m_amt = cmd_inq1.GetDecimal(5);
					price = cmd_inq1.GetDecimal(6);
					use_unit = cmd_inq1.GetDecimal(7);
					m_use_unit = cmd_inq1.GetDecimal(8);
				}
				else
				{
					wt = 0;
					amt = 0;
					m_wt = 0;
					m_amt = 0;
					price = 0;
					use_unit = 0;
					m_use_unit = 0;
				}
				cmd_inq1.Close();
				Log::Trace("", "", "-----sqlstrYJ={0}", sqlstr);
				bcls_ret->Tables[0].Rows[i]["WT"] = wt;
				bcls_ret->Tables[0].Rows[i]["AMT"] = amt;
				bcls_ret->Tables[0].Rows[i]["M_WT"] = m_wt;
				bcls_ret->Tables[0].Rows[i]["M_AMT"] = m_amt;
				bcls_ret->Tables[0].Rows[i]["PRICE"] = price;
				bcls_ret->Tables[0].Rows[i]["USE_UNIT"] = use_unit;
				bcls_ret->Tables[0].Rows[i]["M_USE_UNIT"] = m_use_unit;
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
					sqlstr = " select  t04.mat_code, sum(case when prod_date >= @begin_date and prod_date <= @end_date  then wt else 0 end) wt "
						"   , sum(case when prod_date >= @begin_date  and prod_date <= @end_date THEN cost_GJ "
						" 	ELSE 0 END) amt "
						" 	, sum(wt) m_wt, sum(cost_GJ) m_amt "
						"   ,decode(sum(wt), 0, 0, round(sum(cost_GJ) / sum(wt), 6)) price, "
						"   NVL(use_unit, 0) AS use_unit, NVL(m_use_unit, 0) AS m_use_unit "
						"   from tcaai06 t04"
						" 	left join "
						" 	( "
						" 	select mat_code, decode(sum(case when prod_date >= @begin_date and prod_date <= @end_date  then  wt else 0 end), 0, 0, sum(case when prod_date >= @begin_date and prod_date <= @end_date  then  wt*use_unit else 0 end) / sum(case when prod_date >= @begin_date and prod_date <= @end_date  then  wt else 0 end)) use_unit "
						" 	, decode(sum(wt), 0, 0, ROUND(sum(wt*use_unit) / sum(wt),3)) m_use_unit "
						" 	from tcaai05 t1 "
						" 	left join tcaac05 t2 on t1.sub_backlog_code = t2.sub_backlog_code and t1.product_code = t2.product_code "  //
						//"   left join tcaac05c t2 on t1.sg_sign = t2.sg_sign  and substr(t1.product_code,0,9) = t2.product_code "  //取BOM值
						" 	and  mat_code in(select mat_code from tcaac03 where back_code_2 = @mat_type) "
						" 	where prod_date <= (select max(E_DATETIME) from tcaac15 where S_DATETIME <= @end_date)  and prod_date >= (select max(S_DATETIME) from tcaac15 where S_DATETIME <= @begin_date) "
						"   and t1.sub_backlog_code in('E', 'L', 'R', 'C') "
						" 	group by mat_code ）t3 on t04.mat_code = t3.mat_code "
						" 	where t04.mat_code in(select mat_code from tcaac03 where back_code_2 = @mat_type) "
						"   and prod_date <= (select max(E_DATETIME) from tcaac15 where S_DATETIME <= @end_date) and prod_date >= (select max(S_DATETIME) from tcaac15 where S_DATETIME <= @begin_date) "
						" 	and sub_backlog_code in('E', 'L', 'R', 'C') "
						"   and t04.mat_code=@mat_code "
						" 	group by  t04.mat_code，use_unit，m_use_unit "
						;
					break;
				}
				cmd_inq1.SetCommandText(sqlstr);
				Log::Trace("", "", "-----sqlstrGJ={0}", sqlstr);
				cmd_inq1.Parameters.Set("begin_date", begin_date.SubstringNE(0, 8));
				cmd_inq1.Parameters.Set("end_date", end_date.SubstringNE(0, 8));
				//cmd_inq1.Parameters.Set("sub_backlog_code", sub_backlog_code);
				cmd_inq1.Parameters.Set("mat_type", mat_type);
				cmd_inq1.Parameters.Set("mat_code", mat_code);
				cmd_inq1.ExecuteReader();
				if (cmd_inq1.Read())
				{
					wt = cmd_inq1.GetDecimal(2);
					amt = cmd_inq1.GetDecimal(3);
					m_wt = cmd_inq1.GetDecimal(4);
					m_amt = cmd_inq1.GetDecimal(5);
					price = cmd_inq1.GetDecimal(6);
					use_unit = cmd_inq1.GetDecimal(7);
					m_use_unit = cmd_inq1.GetDecimal(8);
				}
				else
				{
					wt = 0;
					amt = 0;
					m_wt = 0;
					m_amt = 0;
					price = 0;
					use_unit = 0;
					m_use_unit = 0;
				}
				cmd_inq1.Close();
				Log::Trace("", "", "-----sqlstrGJ={0}", sqlstr);
				bcls_ret->Tables[0].Rows[i]["WT"] = wt;
				bcls_ret->Tables[0].Rows[i]["AMT"] = amt;
				bcls_ret->Tables[0].Rows[i]["M_WT"] = m_wt;
				bcls_ret->Tables[0].Rows[i]["M_AMT"] = m_amt;
				bcls_ret->Tables[0].Rows[i]["PRICE"] = price;
				bcls_ret->Tables[0].Rows[i]["USE_UNIT"] = use_unit;
				bcls_ret->Tables[0].Rows[i]["M_USE_UNIT"] = m_use_unit;
				Log::Trace("", "", "END!!!END!!!END!!!END!!!END!!!");
			}

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

