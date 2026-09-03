/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:
Version:    3.0
Date:		2018-06-21
Description:预算产品单价查询，根据设定的各个产品的工艺路径，然后根据各工艺路径经过的工序对应的标准单耗信息进行成本计算
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caaicx34_inq)
//-EP_SYSTEM_HEAD_END
int f_caai_getcb(CString account_period, CString price_terms, CDbConnection * conn);
int f_caaicx34_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount = 0;

	CString		account_period = " ";
	CString		price_terms = " ";
	CDecimal  amt = 0;
	CString   sub_backlog_code = "";
	CString   whole_backlog = "";

	CModel tcaac05b("TCAAC05B");


	CString  sqlstr("");


	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);

	try
	{
		//系统的分页类信息。
		CPageInfo pageInfo;

		tcaac05b.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		account_period = bcls_rec->Tables[0].Rows[0]["ACCOUNT_PERIOD"].ToString().SubstringNE(0, 6);
		price_terms = bcls_rec->Tables[0].Rows[0]["PRICE_TERMS"].ToString();

		/*先查询有哪些生产合同，然后根据生产合同的工艺路径来核算*/
		sqlstr = " delete from tcaac05b"
			" where account_period = @account_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		sqlstr = " insert into tcaac05b(order_no,sg_sign,sg_std,whole_backlog,account_period)"
			" select t1.order_no, sg_sign, sg_std, t1.whole_backlog,@account_period from  tpmof01 t1"
			" left join tom00 t2 on t1.order_no = t2.order_no"
			" where t1.order_status >= '33'"
			" and t1.order_no in(select SALE_ORDER_SUB_NO from tsoso02 where SUBSTR(REC_CREATE_TIME,1,6) = @account_period)"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();


		f_caai_getcb(account_period, price_terms, conn);

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 通用
			sqlstr = " select * "
				" from tcaac05b t "
				" where 1=1"
				;
			if (tcaac05b["PRODUCT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " and product_code = @product_code ";
			}
			if (tcaac05b["PRODUCT_CODE_CNAME"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " and product_code_cname like  '%'||@product_code_cname'%' ";
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

			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("product_code", tcaac05b["PRODUCT_CODE"].ToString());
		cmd_inq.Parameters.Set("product_code_cname", tcaac05b["PRODUCT_CODE_CNAME"].ToString());
		cmd_inq.Parameters.Set("mat_thick", tcaac05b["MAT_THICK"].ToDecimal());
		cmd_inq.Parameters.Set("mat_width", tcaac05b["MAT_WIDTH"].ToDecimal());
		cmd_inq.Parameters.Set("mat_len", tcaac05b["MAT_LEN"].ToDecimal());
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

		/*
		for (int i = 0; i < bcls_ret->Tables[0].Rows.get_Count(); i++)
		{
		whole_backlog = bcls_ret->Tables[0][i]["WHOLE_BACKLOG"].ToString();
		for (int j = 0; j < whole_backlog.GetLength(); j = j + 2)
		{
		amt = 0;
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
		" UNION ALL"  //加工费
		" select BACK_N1 from tcaaib1"
		" where PROJECT_ID = 'JG'"
		" AND BACK_C2  = @sub_backlog_code"
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
		bcls_ret->Tables[0].Rows[i][sub_backlog_code] = cmd_inq_1.GetDecimal(1).Round(4);
		amt = amt + cmd_inq_1.GetDecimal(1);
		}
		cmd_inq_1.Close();
		}
		bcls_ret->Tables[0][i]["PRICE_UNIT"] = amt.Round(4);
		}
		cmd_inq.Close();

		*/


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