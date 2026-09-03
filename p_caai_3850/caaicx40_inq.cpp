/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:
Version:    3.0
Date:		2018-06-21
Description:预算产品单价查询，根据设定的各个产品的工艺路径，然后根据各工艺路径经过的工序对应的标准单耗信息进行成本计算
制造成本 = 炼钢成本/成材率+轧钢成本+后部成本
全成本=制造成本+期间成本
含税全成本=全成本*1.13
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caaicx40_inq)
//-EP_SYSTEM_HEAD_END
int f_caai_matprice(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);
int f_caaicx40_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount = 0;

	CString		account_period = " ";
	CString		price_terms = " ";
	CDecimal  amt = 0;
	CString   sub_backlog_code = "";
	CString   whole_backlog = "";
	CString   whole_backlog_desc = "";

	CModel tcaac05b("TCAAC05B");

	int  v_total_count = 0;
	//系统的分页类信息。
	CPageInfo pageInfo;


	CString  sqlstr("");


	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);

	try
	{
		//系统的分页类信息。
		try
		{
			pageInfo.MergeFrom(bcls_rec->Tables["PageInfo"].Rows[0]);
		}
		catch (CException& ce)
		{
			pageInfo.RecordFrom = 0;
			pageInfo.PageSize = 5000;
		}

		tcaac05b.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		account_period = bcls_rec->Tables[0].Rows[0]["ACCOUNT_PERIOD"].ToString().SubstringNE(0, 6);
		price_terms = bcls_rec->Tables[0].Rows[0]["PRICE_TERMS"].ToString();

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 通用
			sqlstr = " select t.*"
				",ROUND(A1_COST+ALLOY_COST+GTL_COST+RH_COST,2) as S_ALL, ROUND(B0_COST+B1_COST+B2_COST+B3_COST+D1_COST,2) AS X_ALL"
				", ROUND(H1_COST+H2_COST+H3_COST+H4_COST+L1_COST+S0_COST+T1_COST+T2_COST+T3_COST+T4_COST+T5_COST+T6_COST+T7_COST+Y1_COST,2) AS R_ALL"
				",ROUND(PRICE_UNIT-QJ_COST,2) AS ZZ_ALL,ROUND(PRICE_UNIT,2) AS ALL_COST,ROUND(PRICE_UNIT*1.13,2) AS ALL_HS_COST"
				" from tcaac05b t "
				" where 1=1"
				//" and account_period = @account_period"
				;
			if (tcaac05b["PRODUCT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " and product_code like  '%'||@product_code||'%' ";
			}
			if (tcaac05b["PRODUCT_CODE_CNAME"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " and product_code_cname like  '%'||@product_code_cname||'%' ";
			}
			if (tcaac05b["SG_SIGN"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " and SG_SIGN like  '%'||@sg_sign||'%' ";
			}
			if (tcaac05b["MAT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " and MAT_CODE like  '%'||@mat_code||'%' ";
			}
			if (tcaac05b["MAT_THICK"].ToDecimal() != 0)
			{
				sqlstr = sqlstr + " and mat_thick = @mat_thick ";
			}
			if (tcaac05b["MAT_WIDTH"].ToDecimal() != 0)
			{
				sqlstr = sqlstr + " and mat_width = @mat_width ";
			}
			if (tcaac05b["MAT_LEN"].ToDecimal() != 0)
			{
				sqlstr = sqlstr + " and mat_len = @mat_len ";
			}

			sqlstr = sqlstr + " and price_terms =@price_terms and account_period = @account_period ";

			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("product_code", tcaac05b["PRODUCT_CODE"].ToString());
		cmd_inq.Parameters.Set("product_code_cname", tcaac05b["PRODUCT_CODE_CNAME"].ToString());
		cmd_inq.Parameters.Set("sg_sign", tcaac05b["SG_SIGN"].ToString());
		cmd_inq.Parameters.Set("mat_code", tcaac05b["MAT_CODE"].ToString());
		cmd_inq.Parameters.Set("mat_thick", tcaac05b["MAT_THICK"].ToDecimal());
		cmd_inq.Parameters.Set("mat_width", tcaac05b["MAT_WIDTH"].ToDecimal());
		cmd_inq.Parameters.Set("mat_len", tcaac05b["MAT_LEN"].ToDecimal());
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("price_terms", price_terms);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0], pageInfo.RecordFrom, pageInfo.PageSize);
		cmd_inq.Close();

		sqlstr = "SELECT count(1) FROM tcaac05b"
			" WHERE  1=1 "
			;
		if (tcaac05b["PRODUCT_CODE"].ToString().Trim() != "")
		{
			sqlstr = sqlstr + " and product_code like  '%'||@product_code||'%' ";
		}
		if (tcaac05b["PRODUCT_CODE_CNAME"].ToString().Trim() != "")
		{
			sqlstr = sqlstr + " and product_code_cname like  '%'||@product_code_cname||'%' ";
		}
		if (tcaac05b["SG_SIGN"].ToString().Trim() != "")
		{
			sqlstr = sqlstr + " and SG_SIGN like  '%'||@sg_sign||'%' ";
		}
		if (tcaac05b["MAT_CODE"].ToString().Trim() != "")
		{
			sqlstr = sqlstr + " and MAT_CODE like  '%'||@mat_code||'%' ";
		}
		if (tcaac05b["MAT_THICK"].ToDecimal() != 0)
		{
			sqlstr = sqlstr + " and mat_thick = @mat_thick ";
		}
		if (tcaac05b["MAT_WIDTH"].ToDecimal() != 0)
		{
			sqlstr = sqlstr + " and mat_width = @mat_width ";
		}
		if (tcaac05b["MAT_LEN"].ToDecimal() != 0)
		{
			sqlstr = sqlstr + " and mat_len = @mat_len ";
		}

		sqlstr = sqlstr + " and price_terms =@price_terms and account_period = @account_period ";

	cmd_inq.SetCommandText(sqlstr);
	cmd_inq.Parameters.Set("product_code", tcaac05b["PRODUCT_CODE"].ToString());
	cmd_inq.Parameters.Set("product_code_cname", tcaac05b["PRODUCT_CODE_CNAME"].ToString());
	cmd_inq.Parameters.Set("sg_sign", tcaac05b["SG_SIGN"].ToString());
	cmd_inq.Parameters.Set("mat_code", tcaac05b["MAT_CODE"].ToString());
	cmd_inq.Parameters.Set("mat_thick", tcaac05b["MAT_THICK"].ToDecimal());
	cmd_inq.Parameters.Set("mat_width", tcaac05b["MAT_WIDTH"].ToDecimal());
	cmd_inq.Parameters.Set("mat_len", tcaac05b["MAT_LEN"].ToDecimal());
	cmd_inq.Parameters.Set("account_period", account_period);
	cmd_inq.Parameters.Set("price_terms", price_terms);
	v_total_count = cmd_inq.ExecuteScalar().ToInt32();
	cmd_inq.Close();

	bcls_ret->Tables.Add("PageInfo");
	bcls_ret->Tables["PageInfo"].Columns.Add(DT_INT32, "TotalRecordCount");
	bcls_ret->Tables["PageInfo"].Rows.Add();
	bcls_ret->Tables["PageInfo"].Rows[0]["TotalRecordCount"] = v_total_count;

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