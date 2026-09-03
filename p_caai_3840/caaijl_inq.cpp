/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2019
Author:      admin
Version:     1.0
Date:        2019-02-26 13:40:26
Description: 厂内计量查询
**************************************************/

#include "stdafx.h"

BM2F_ENTERACE(caaijl_inq)


int f_caaijl_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CDbCommand cmd_inq(conn);
	CString sqlstr = " ";

	CString begin_time = "";
	CString end_time = "";
	CString bill_no = "";
	CString SRC_MAT_CNAME = "";
	CString busi_type = "";

	int  v_total_count = 0;
	//系统的分页类信息。
	CPageInfo pageInfo;

	CModel tymrec1fg("TYMREC1FG");

	try
	{
		////获取前台DEV控件传入的分页信息，2个变量信息是由前台的分页控件信息传入的，获取失败时,人工赋值一下。
		try
		{
			pageInfo.MergeFrom(bcls_rec->Tables["PageInfo"].Rows[0]);
		}
		catch (CException& ce)
		{
			pageInfo.RecordFrom = 0;
			pageInfo.PageSize = 10000;
		}
		
		begin_time = bcls_rec->Tables[0].Rows[0]["START_DATE"].ToString().SubstringNE(0,8);
		end_time = bcls_rec->Tables[0].Rows[0]["END_DATE"].ToString().SubstringNE(0, 8);
		tymrec1fg.MergeFrom(bcls_rec->Tables[0].Rows[0]);
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
		}*/
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 通用
			sqlstr = "SELECT WEIGH_APP_NO, "  //磅单号,
				" WEIGH_APP_TIME, "//计量时刻，
				" DST_MAT_CODE, "//物料编码,
				" DST_MAT_CNAME, " //物料名称
				" NET_WT,    "     //重量,
				" SRC_STOCK_CODE, "//源库区,
				" DST_STOCK_CODE, "//目的库区
				" TRUCK_NO, " //车号
				" WEIGH_BY, " //司磅员
				" WEIGH_SITE, "  //磅站
				" RECV_CONFM_FLAG "  //收料确认标记
				" FROM tymrec1fg t "
				" WHERE WEIGH_APP_TYPE = 'HF' "; //--回收废钢 			
				;
			 
			if (begin_time.Trim() != "")
			{
				sqlstr = sqlstr + " AND WEIGH_APP_TIME >= @begin_time";
			}
			if (end_time.Trim() != "")
			{
				sqlstr = sqlstr + " AND WEIGH_APP_TIME <= @end_time";
			}
			if (tymrec1fg["DST_MAT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND DST_MAT_CODE LIKE  '%'||@dst_mat_code||'%' ";
			}
			if (tymrec1fg["DST_MAT_CNAME"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND DST_MAT_CNAME  like '%'||@dst_mat_cname||'%'";
			}
			if (tymrec1fg["SRC_STOCK_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND SRC_STOCK_CODE  like '%'||@src_stock_code||'%'";
			}
			if (tymrec1fg["WEIGH_APP_NO"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND WEIGH_APP_NO  like '%'||@weigh_app_no||'%'";
			}
			if (tymrec1fg["RECV_CONFM_FLAG"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND RECV_CONFM_FLAG=@recv_confm_flag ";
			}
			/*if (tcaais6["WEIGH_NO"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND WEIGH_NO like '%'||@weigh_no||'%' ";
			}
			
			if (tcaais6["TRUCK_NO"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND TRUCK_NO  = @truck_no ";
			}
			if (tcaais6["COST_CENTER"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND COST_CENTER  = @cost_center ";
			}*/
			break;
		}

		cmd_inq.SetCommandText(sqlstr);

		cmd_inq.Parameters.Set("begin_time", begin_time);
		cmd_inq.Parameters.Set("end_time", end_time);		
		cmd_inq.Parameters.Set("weigh_app_no", tymrec1fg["WEIGH_APP_NO"].ToString());
		cmd_inq.Parameters.Set("dst_mat_code", tymrec1fg["DST_MAT_CODE"].ToString());
		cmd_inq.Parameters.Set("dst_mat_cname", tymrec1fg["DST_MAT_CNAME"].ToString());
		cmd_inq.Parameters.Set("src_stock_code", tymrec1fg["SRC_STOCK_CODE"].ToString());
		cmd_inq.Parameters.Set("recv_confm_flag", tymrec1fg["RECV_CONFM_FLAG"].ToString());
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
			sqlstr = "SELECT count(1) FROM tymrec1fg t "
				" WHERE WEIGH_APP_TYPE = 'HF' "
				;

			if (begin_time.Trim() != "")
			{
				sqlstr = sqlstr + " AND WEIGH_APP_TIME >= @begin_time";
			}
			if (end_time.Trim() != "")
			{
				sqlstr = sqlstr + " AND WEIGH_APP_TIME <= @end_time";
			}
			if (tymrec1fg["DST_MAT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND DST_MAT_CODE LIKE  '%'||@dst_mat_code||'%' ";
			}
			if (tymrec1fg["DST_MAT_CNAME"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND DST_MAT_CNAME  like '%'||@dst_mat_cname||'%'";
			}
			if (tymrec1fg["SRC_STOCK_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND SRC_STOCK_CODE  like '%'||@src_stock_code||'%'";
			}
			if (tymrec1fg["WEIGH_APP_NO"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND WEIGH_APP_NO  like '%'||@weigh_app_no||'%'";
			}
			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("begin_time", begin_time);
		cmd_inq.Parameters.Set("end_time", end_time);
		cmd_inq.Parameters.Set("weigh_app_no", tymrec1fg["WEIGH_APP_NO"].ToString());
		cmd_inq.Parameters.Set("dst_mat_code", tymrec1fg["DST_MAT_CODE"].ToString());
		cmd_inq.Parameters.Set("dst_mat_cname", tymrec1fg["DST_MAT_CNAME"].ToString());
		cmd_inq.Parameters.Set("src_stock_code", tymrec1fg["SRC_STOCK_CODE"].ToString());
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


