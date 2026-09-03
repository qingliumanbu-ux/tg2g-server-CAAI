/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhouli
Version:    1.0
Date:     2013-03-19
Description: 生产计产时间点
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 交易资料需检索的信息查询
/// <para>
/// 1.根据输入参数查询相应信息。
///
/// <para>数据库表：tcaaia14(交易资料明细表)         </para>
/// </summary>
/// <param name="ACCOUNT_PERIOD">帐务会计期    </param>
/// <param name="line_type">产线  </param>
/// <returns>交易资料需检索的信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(caai14_inq)

int f_caai14_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义
	CDbCommand cmd(conn);
	CString  sqlstr("");
	CString v_line_type("");

	CString begin_time("");
	CString end_time("");

	int  v_total_count = 0;
	//系统的分页类信息。
	CPageInfo pageInfo;


	CDbCommand cmd_inq(conn);
	CModel tcaaia14("TCAAIA14");

	/* ***** 静态变量定义 ***** */
	int 	doFlag = 0;
	int 	fetchRowCount = 0;
	int     RowCount = 0;

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
		EDLog(1, 1, "pageInfo.RecordFrom[%d]pageInfo.PageSize[%d]", pageInfo.RecordFrom, pageInfo.PageSize);

		begin_time = bcls_rec->Tables[0].Rows[0]["BEGIN_TIME"].ToString().SubstringNE(0, 8);
		end_time = bcls_rec->Tables[0].Rows[0]["END_TIME"].ToString().SubstringNE(0, 8);
		tcaaia14.MergeFrom(bcls_rec->Tables[0].Rows[0]);


		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 通用
			sqlstr = "SELECT * FROM tcaaia14"
				" WHERE  1=1 "
				;			
			
			if (tcaaia14["SUB_BACKLOG_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND SUB_BACKLOG_CODE=@sub_backlog_code";
			}
			if (tcaaia14["HEAT_NO"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND HEAT_NO like '%'||@heat_no||'%' ";
			}
			if (tcaaia14["RELATION_NO"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND RELATION_NO like '%'||@relation_no||'%' ";
			}
			if (tcaaia14["EQU_NO"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND EQU_NO =@equ_no ";
			}			
			if (tcaaia14["PLAN_NO"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND PLAN_NO like '%'||@plan_no||'%' ";
			}
			if (tcaaia14["DEPT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND DEPT_CODE=@dept_code";
			}
			if (tcaaia14["ENTRUST_FLAG"].ToString().Trim() == "1")
			{
				sqlstr = sqlstr + " AND ENTRUST_FLAG='1'";
			}
			else
			{
				sqlstr = sqlstr + " AND ENTRUST_FLAG!='1'";
			}
			
			if (begin_time.Trim() != "")
				sqlstr = sqlstr + " AND stats_period>=@begin_time";
			if (end_time.Trim() != "")
				sqlstr = sqlstr + " AND stats_period<=@end_time ";

			sqlstr = sqlstr + " ORDER BY stats_period,dept_code,SUB_BACKLOG_CODE,RELATION_NO  ";

			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", "", "sqlstr={0}", sqlstr);
		cmd_inq.Parameters.Set("begin_time", begin_time);
		cmd_inq.Parameters.Set("end_time", end_time);
		cmd_inq.Parameters.Set("sub_backlog_code", tcaaia14["SUB_BACKLOG_CODE"]);
		cmd_inq.Parameters.Set("heat_no", tcaaia14["HEAT_NO"]);
		cmd_inq.Parameters.Set("relation_no", tcaaia14["RELATION_NO"]);
		cmd_inq.Parameters.Set("equ_no", tcaaia14["EQU_NO"]);
		cmd_inq.Parameters.Set("plan_no", tcaaia14["PLAN_NO"]);
		cmd_inq.Parameters.Set("dept_code", tcaaia14["DEPT_CODE"]);
		cmd_inq.Parameters.Set("entrust_flag", tcaaia14["ENTRUST_FLAG"].ToString());
		//定义返回的表名
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
			sqlstr = "SELECT count(1) FROM tcaaia14"
				" WHERE  1=1 "
				;			
			if (tcaaia14["SUB_BACKLOG_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND SUB_BACKLOG_CODE=@sub_backlog_code";
			}
			if (tcaaia14["HEAT_NO"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND HEAT_NO like '%'||@heat_no||'%'";
			}
			if (tcaaia14["EQU_NO"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND EQU_NO =@equ_no ";
			}			
			if (tcaaia14["RELATION_NO"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND RELATION_NO like '%'||@relation_no||'%' ";
			}
			if (tcaaia14["PLAN_NO"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND PLAN_NO like '%'||@plan_no||'%' ";
			}
			if (tcaaia14["DEPT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND DEPT_CODE=@dept_code";
			}
			if (tcaaia14["ENTRUST_FLAG"].ToString().Trim() == "1")
			{
				sqlstr = sqlstr + " AND ENTRUST_FLAG='1'";
			}
			else
			{
				sqlstr = sqlstr + " AND ENTRUST_FLAG!='1'";
			}
			if (begin_time.Trim() != "")
				sqlstr = sqlstr + " AND stats_period>=@begin_time";
			if (end_time.Trim() != "")
				sqlstr = sqlstr + " AND stats_period<=@end_time ";


			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("begin_time", begin_time);
		cmd_inq.Parameters.Set("end_time", end_time);
		cmd_inq.Parameters.Set("sub_backlog_code", tcaaia14["SUB_BACKLOG_CODE"]);
		cmd_inq.Parameters.Set("heat_no", tcaaia14["HEAT_NO"]);
		cmd_inq.Parameters.Set("equ_no", tcaaia14["EQU_NO"]);
		cmd_inq.Parameters.Set("relation_no", tcaaia14["RELATION_NO"]);
		cmd_inq.Parameters.Set("plan_no", tcaaia14["PLAN_NO"]);
		cmd_inq.Parameters.Set("dept_code", tcaaia14["DEPT_CODE"]);
		cmd_inq.Parameters.Set("entrust_flag", tcaaia14["ENTRUST_FLAG"].ToString());
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

