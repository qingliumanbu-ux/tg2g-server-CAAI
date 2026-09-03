/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2018
Author:      admin
Version:     1.0
Date:        2018-12-25 08:47:27
Description: caaimm
**************************************************/

#include "stdafx.h"
BM2F_ENTERACE(caaimm_inq)


int f_caaimm_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CDbCommand cmd(conn);
	CString sqlstr = " ";

	CString begin_time = "";
	CString end_time = "";
	CString factory_div= " ";
	CString whole_backlog_code = " ";
	CString prod_code = "";
	CString acjc_relation_id = "";
	CString sg_sign = "";
	CString mat_thick = "";
	CString deal_flag = "";
	CDbCommand cmd_inq(conn);

	int  v_total_count = 0;
	//系统的分页类信息。
	CPageInfo pageInfo;

	CModel tmm00ac("TMM00AC");

	try
	{
		////获取前台DEV控件传入的分页信息，2个变量信息是由前台的分页控件信息传入的，获取失败时,人工赋值一下。
		try
		{
			pageInfo.MergeFrom(bcls_rec->Tables["PageInfo"].Rows[0]);
			pageInfo.RecordFrom = 0;
			pageInfo.PageSize = 10000;
		}
		catch (CException& ce)
		{
			pageInfo.RecordFrom = 0;
			pageInfo.PageSize = 10000;
		}
		EDLog(1, 1, "pageInfo.RecordFrom[%d]pageInfo.PageSize[%d]", pageInfo.RecordFrom, pageInfo.PageSize);

		begin_time = bcls_rec->Tables[0].Rows[0]["BEGIN_TIME"].ToString().Substring(0, 8);
		end_time = bcls_rec->Tables[0].Rows[0]["END_TIME"].ToString().Substring(0, 8);

		tmm00ac.MergeFrom(bcls_rec->Tables[0].Rows[0]);		
		//日期都存在判断范围一个月内
		/*if ((begin_time.Trim() != "") && (end_time.Trim() != ""))
		{
			CDateTime d1 = CDateTime::Parse(begin_time);
			CDateTime d2 = CDateTime::Parse(end_time);
			CTimeSpan timeSpan = d2 - d1;
			if (timeSpan.Days() > 30)
			{
				throw CApplicationException("日期范围超过一个月！");
				
			}
		}		*/
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 通用
			sqlstr = "SELECT * FROM tmm00ac"
				"  left join tcaac11 on tmm00ac.PRODUCT_CODE=tcaac11.mat_code "
				" WHERE  1=1 "
				;		
			
			if (tmm00ac["PRODUCT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND PRODUCT_CODE = @product_code ";
			}
			if (tmm00ac["ACJC_RELATION_ID"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND ACJC_RELATION_ID = @acjc_relation_id ";

			}
			if (tmm00ac["WHOLE_BACKLOG_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND WHOLE_BACKLOG_CODE = @whole_backlog_code ";
			}
			/*if (tmm00ac["TRANSACTION_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND TRANSACTION_CODE = @transaction_code ";
			}*/
			if (tmm00ac["SG_SIGN"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND SG_SIGN = @sg_sign ";
			}
			if (tmm00ac["MAT_THICK"].ToDecimal() != 0)
			{
				sqlstr = sqlstr + " AND MAT_THICK = @mat_thick ";
			}
			if (tmm00ac["TRANSACTION_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND TRANSACTION_CODE = @transaction_code ";
			}
			if (tmm00ac["FACTORY_DIV"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND FACTORY_DIV = @factory_div";
			}			
			if (begin_time.Trim() != "")
			{
				sqlstr = sqlstr + " AND BALANCE_DATE >= @begin_time";
			}
			if (end_time.Trim() != "")
			{
				sqlstr = sqlstr + " AND BALANCE_DATE <= @end_time";
			}
			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		/*Log::Trace("", "", "WHOLE_BACKLOG_CODE={0}", whole_backlog_code);
		Log::Trace("", "", "FACTORY_DIV={0}", factory_div);
		Log::Trace("", "", "begin_time={0}", begin_time);
		Log::Trace("", "", "end_time={0}", end_time);*/
		

		cmd_inq.Parameters.Set("factory_div", tmm00ac["FACTORY_DIV"].ToString());
		cmd_inq.Parameters.Set("begin_time", begin_time);
		cmd_inq.Parameters.Set("end_time", end_time);

		cmd_inq.Parameters.Set("product_code", tmm00ac["PRODUCT_CODE"].ToString());
		cmd_inq.Parameters.Set("acjc_relation_id", tmm00ac["ACJC_RELATION_ID"].ToString());

		cmd_inq.Parameters.Set("whole_backlog_code", tmm00ac["WHOLE_BACKLOG_CODE"].ToString());
		//cmd_inq.Parameters.Set("transaction_code", tmm00ac["TRANSACTION_CODE"].ToString());

		cmd_inq.Parameters.Set("sg_sign", tmm00ac["SG_SIGN"].ToString());
		cmd_inq.Parameters.Set("mat_thick", tmm00ac["MAT_THICK"].ToDecimal());
		cmd_inq.Parameters.Set("transaction_code", tmm00ac["TRANSACTION_CODE"].ToString());
		//Log::Trace("", "", "-----sqlstr={0}", sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0], pageInfo.RecordFrom, pageInfo.PageSize);
		cmd_inq.Close();
		/// <summary>
		/// 返回总记录数
		/// </summary>  
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 通用
			sqlstr = "SELECT count(1) FROM tmm00ac"
				"  left join tcaac11 on tmm00ac.PRODUCT_CODE=tcaac11.mat_code "
				" WHERE  1=1 "
				;
			if (tmm00ac["PRODUCT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND PRODUCT_CODE = @product_code ";
			}
			if (tmm00ac["ACJC_RELATION_ID"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND ACJC_RELATION_ID = @acjc_relation_id ";
			}
			if (tmm00ac["WHOLE_BACKLOG_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND WHOLE_BACKLOG_CODE = @whole_backlog_code ";
			}
		/*	if (tmm00ac["TRANSACTION_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND TRANSACTION_CODE = @transaction_code ";
			}*/
			if (tmm00ac["SG_SIGN"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND SG_SIGN = @sg_sign ";
			}
			if (tmm00ac["MAT_THICK"].ToDecimal() != 0)
			{
				sqlstr = sqlstr + " AND MAT_THICK = @mat_thick ";
			}
			if (tmm00ac["TRANSACTION_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND TRANSACTION_CODE = @transaction_code ";
			}
			if (tmm00ac["FACTORY_DIV"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND FACTORY_DIV = @factory_div";
			}
			
			if (begin_time.Trim() != "")
			{
				sqlstr = sqlstr + " AND BALANCE_DATE >= @begin_time";
			}
			if (end_time.Trim() != "")
			{
				sqlstr = sqlstr + " AND BALANCE_DATE <= @end_time";
			}
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		
		//Log::Trace("", "", "sqlstr={0}", sqlstr);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			v_total_count = cmd_inq.GetInt32(1);
		}
		bcls_ret->Tables.Add("PageInfo");
		bcls_ret->Tables["PageInfo"].Columns.Add(DT_INT32, "TotalRecordCount");
		bcls_ret->Tables["PageInfo"].Rows.Add();
		bcls_ret->Tables["PageInfo"].Rows[0]["TotalRecordCount"] = v_total_count;

		cmd_inq.Close();

	}
	catch (CDbException& ex)
	{
		CFormattable arguments[] = { ex.GetCode(), ex.GetMsg() };
		CMessageFormat::Format(s.msg, "Database Error,sqlcode=[{0}],sqlmsg=[{1}]", arguments, 2);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;
	}
	catch (CApplicationException& ex)
	{
		s.flag = ex.GetCode();
		strcpy(s.msg, ex.GetMsg());
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strcpy(s.msg, ex.GetMsg());
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}


