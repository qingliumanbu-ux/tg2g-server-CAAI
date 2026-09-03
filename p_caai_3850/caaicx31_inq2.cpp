/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    3.0
Date:		2012-2-9
Description:查询热轧煤耗的明细信息
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caaicx31_inq2)
//-EP_SYSTEM_HEAD_END

int f_caaicx31_inq2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount = 0;
	CModel tcaac05a("TCAAC05A");

	
	CString   dept_code = "";
	CString   account_period = "";
	CString   price_terms = "";
	CString   sub_backlog_code = "";
	CString   whole_backlog = "";

	CString  sqlstr("");

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);

	try
	{
		tcaac05a.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		account_period = bcls_rec->Tables[1].Rows[0]["ACCOUNT_PERIOD"].ToString().SubstringNE(0, 6);		
		price_terms = bcls_rec->Tables[1].Rows[0]["PRICE_TERMS"].ToString();

		
		whole_backlog = tcaac05a["WHOLE_BACKLOG"].ToString();
		sub_backlog_code = "'";		
		for (int j = 0; j < whole_backlog.GetLength(); j = j + 2)
		{
			sub_backlog_code = sub_backlog_code + "','" + whole_backlog.SubstringNE(j, 2);
		}
		sub_backlog_code = sub_backlog_code + "'";

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句		

			//每个工序对应的消耗量及分摊量
			sqlstr = " select t1.mat_code,t3.mat_name,round(nvl(t2.price_unit,0),4) as PRICE_UNIT,sub_backlog_code,nvl(t1.use_unit*@out_wt,0) AS WT,round(nvl(t1.use_unit*@out_wt*t2.price_unit,0),4) as AMT from tcaac05 t1"
				" left join"
				" (SELECT mat_code, price_unit from  tcaac12 where(mat_code, VALID_TIME_START) in"
				" (select mat_code, max(VALID_TIME_START) from tcaac12"
				" where VALID_TIME_START <= @account_period and PRICE_TERMS = @price_terms group by mat_code, price_unit)"
				" and  PRICE_TERMS = @price_terms"
				" ) t2 on t1.mat_code = t2.mat_code"
				" left join tcaac11 t3 on t1.mat_code = t3.mat_code"
				" where sub_backlog_code in  ("+sub_backlog_code+")"
				" and product_code = @product_code"
				
				;
			break;			
		}
		//Log::Trace("", __FUNCTION__, "sub_backlog_code = [{0}],product_code=[{1}],out_wt =[{2}] , account_period=[{3}]", sub_backlog_code, tcaac05a["PRODUCT_CODE"].ToString(), tcaac05a["WT"].ToDecimal(), account_period);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("price_terms", price_terms);
		//cmd_inq.Parameters.Set("sub_backlog_code", sub_backlog_code);
		cmd_inq.Parameters.Set("product_code", tcaac05a["PRODUCT_CODE"].ToString() );
		cmd_inq.Parameters.Set("out_wt", tcaac05a["WT"].ToDecimal());
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应

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

	s.flag = doFlag;
	bcls_ret->SetSYS(s);
	return doFlag;
}