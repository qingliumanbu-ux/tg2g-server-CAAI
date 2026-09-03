/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhl
Version:    1.0
Date:     2016-03-10
Description: 工序收集规则查询
**************************************************/

#include "stdafx.h"

// Service 入口
BM2F_ENTERACE(caac03_1_inq)

int f_caac03_1_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	//APP_BEGIN()
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义
	int doFlag = 0;

	CString sqlstr("");
	CString sqlstr1("");
	CString cname("");
	CString mat_type("");
	CString sub_backlog_code("");
	CString mat_code("");
	CString mat_name("");
	CString dept_code("");
	/* 实体类定义 */
	CModel tcaac11("TCAAC11");
	CDbCommand cmd_inq(conn);

	try
	{
		/* 获得输入参数 */
		tcaac11.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		//dept_code = bcls_rec->Tables[0].Rows[0]["DEPT_CODE"].ToString();
		sub_backlog_code = bcls_rec->Tables[0].Rows[0]["SUB_BACKLOG_CODE"].ToString();
		cname = bcls_rec->Tables[0].Rows[0]["CNAME"].ToString();
		dept_code = bcls_rec->Tables[0].Rows[0]["DEPT_CODE"].ToString();
		Log::Trace("", __FUNCTION__, "sub_backlog_code[{0}]", sub_backlog_code);
		Log::Trace("", __FUNCTION__, "cname[{0}]", cname);
		Log::Trace("", __FUNCTION__, "dept_code[{0}]", dept_code);
		/*mat_type = bcls_rec->Tables[0].Rows[0]["MAT_TYPE"].ToString();
		sub_backlog_code = bcls_rec->Tables[0].Rows[0]["SUB_BACKLOG_CODE"].ToString();
		mat_code = bcls_rec->Tables[0].Rows[0]["MAT_CODE"].ToString();
		mat_name = bcls_rec->Tables[0].Rows[0]["MAT_NAME"].ToString();*/

		switch (conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				/*sqlstr = " SELECT * "
					" FROM tcaac03 t"
					" WHERE 1=1";

				if (mat_type.Trim() != "")sqlstr = "select t.mat_code,t.mat_name from tcaac03 t left join tcaac07 s on t.mat_code=s.mat_code where s.mat_type=@mat_type ";
				if (sub_backlog_code.Trim() != "")sqlstr = sqlstr + " AND t.SUB_BACKLOG_CODE = @sub_backlog_code";
				if (mat_code.Trim() != "") sqlstr = sqlstr + " AND t.MAT_CODE like @mat_code||'%'";
				if (mat_name.Trim() != "") sqlstr = sqlstr + " AND t.MAT_NAME like '%'||@mat_name||'%'";
				if (dept_code.Trim() != "")sqlstr = sqlstr + sqlstr1 + " AND t.BACK_CODE_1=@dept_code ";
				sqlstr = sqlstr + " ORDER BY t.SCHE_NO";*/

				sqlstr = "select @sub_backlog_code as SUB_BACKLOG_CODE,@cname as ITEM_CNAME , MAT_CODE,MAT_NAME,@dept_code as BACK_CODE_1 from tcaac11  where MAT_CODE not in(select mat_code from tcaac03 "
						 "where mat_code=tcaac11.mat_code and sub_backlog_code=@sub_backlog_code) ";

				if (tcaac11["MAT_CODE"].ToString().Trim() != "") sqlstr = sqlstr + " AND MAT_CODE like @mat_code||'%'";
				if (tcaac11["MAT_NAME"].ToString().Trim() != "") sqlstr = sqlstr + " AND MAT_NAME like '%'||@mat_name||'%'";
				if (tcaac11["MAT_TYPE"].ToString().Trim() != "") sqlstr = sqlstr + " AND MAT_TYPE like @mat_type||'%'";
				//if (dept_code.Trim() != "")sqlstr = sqlstr + " AND BACK_CODE_1=@dept_code ";
			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		/*cmd_inq.Parameters.Set("mat_code", mat_code);
		cmd_inq.Parameters.Set("mat_name", mat_name);
		cmd_inq.Parameters.Set("sub_backlog_code", sub_backlog_code);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("mat_type", mat_type);*/
		cmd_inq.Parameters.Set("sub_backlog_code",sub_backlog_code);
		cmd_inq.Parameters.Set("mat_code", tcaac11["MAT_CODE"]);
		cmd_inq.Parameters.Set("mat_name", tcaac11["MAT_NAME"]);
		cmd_inq.Parameters.Set("mat_type", tcaac11["MAT_TYPE"]);
		cmd_inq.Parameters.Set("cname", cname);
		cmd_inq.Parameters.Set("dept_code", dept_code);

		Log::Trace("", __FUNCTION__, "sqlstr[{0}]", sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.msg, (const char*)str, sizeof(s.msg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
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
