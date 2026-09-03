/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		
Version:    3.0
Date:		2020-04-09
Description:计划成本查询主体
**************************************************/
//框架用头文件
#include "stdafx.h"


// service入口
BM2F_ENTERACE(caaicx31_inq)
//-EP_SYSTEM_HEAD_END

int f_caaicx31_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	
	int doFlag			= 0;
	int fetchRowCount	= 0;

	CString   dept_code = "";
	CString   account_period = "";
	CString   price_terms = "";
	CString   sub_backlog_code = "";
	CString   whole_backlog = "";

	CModel tcaac05a("TCAAC05A");

	CString  sqlstr("");

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);
	
	try
	{
		//系统的分页类信息。
		CPageInfo pageInfo;

		tcaac05a.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		account_period = bcls_rec->Tables[0].Rows[0]["ACCOUNT_PERIOD"].ToString().SubstringNE(0, 6);
		dept_code = bcls_rec->Tables[0].Rows[0]["DEPT_CODE"].ToString();
		price_terms = bcls_rec->Tables[0].Rows[0]["PRICE_TERMS"].ToString();


		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 通用
			sqlstr = " select * "
				" from tcaac05a t "
				" where 1=1"
				" and account_period = @account_period "
				;
			if (tcaac05a["DEPT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " and dept_code = @dept_code ";
			}
			if (tcaac05a["MAT_THICK"].ToDecimal() != 0)
			{
				sqlstr = sqlstr + " and mat_thick = @mat_thick ";
			}
			if (tcaac05a["MAT_WIDTH"].ToDecimal() != 0)
			{
				sqlstr = sqlstr + " and mat_width = @mat_width ";
			}
			if (tcaac05a["MAT_LEN"].ToDecimal() != 0)
			{
				sqlstr = sqlstr + " and mat_len = @mat_len ";
			}

			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("mat_thick", tcaac05a["MAT_THICK"].ToDecimal());
		cmd_inq.Parameters.Set("mat_width", tcaac05a["MAT_WIDTH"].ToDecimal());
		cmd_inq.Parameters.Set("mat_len", tcaac05a["MAT_LEN"].ToDecimal());
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

		for (int i = 0; i < bcls_ret->Tables[0].Rows.get_Count(); i++)
		{
			whole_backlog = bcls_ret->Tables[0][i]["WHOLE_BACKLOG"].ToString();
			for (int j = 0; j < whole_backlog.GetLength(); j = j + 2)
			{
				sub_backlog_code = whole_backlog.SubstringNE(j, 2);
				//每个工序对应的消耗量及分摊量
				sqlstr = " select sum(use_unit*price_unit) from ("
					" select t1.mat_code,t1.use_unit,t2.price_unit from tcaac05 t1"
					" left join"
					" (SELECT mat_code, price_unit from  tcaac12 where(mat_code, VALID_TIME_START) in"
					" (select mat_code, max(VALID_TIME_START) from tcaac12"
					" where VALID_TIME_START <= @account_period and PRICE_TERMS = @price_terms group by mat_code, price_unit)"
					" and  PRICE_TERMS = @price_terms"
					" ) t2 on t1.mat_code = t2.mat_code"
					" where sub_backlog_code = @sub_backlog_code"
					" and product_code =@product_code"
					")"
					;
				Log::Trace("", __FUNCTION__, "sub_backlog_code = [{0}],sqlstr=[{1}]  ", sub_backlog_code, sqlstr);
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("account_period", account_period);
				cmd_inq_1.Parameters.Set("price_terms", price_terms);
				cmd_inq_1.Parameters.Set("sub_backlog_code", sub_backlog_code);
				cmd_inq_1.Parameters.Set("product_code", bcls_ret->Tables[0][i]["PRODUCT_CODE"].ToString());
				cmd_inq_1.ExecuteReader();
				while (cmd_inq_1.Read())
				{
					sub_backlog_code = sub_backlog_code + "_COST";
					if (!bcls_ret->Tables[0].Columns.Contains(sub_backlog_code))
					{
						bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, sub_backlog_code);
					}
					bcls_ret->Tables[0].Rows[i][sub_backlog_code] = (cmd_inq_1.GetDecimal(1)*bcls_ret->Tables[0].Rows[i]["WT"].ToDecimal()).Round(2);
				}
				cmd_inq_1.Close();
			}

		}
		cmd_inq.Close();


	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		//Log::Trace("", "", "sqlstr={0}", sqlstr);
		//Log::Trace("", "", "str={0}", str);
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
	
	s.flag = doFlag;
	bcls_ret->SetSYS(s);
	return doFlag;
}