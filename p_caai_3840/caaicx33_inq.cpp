/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhouli
Version:    1.0
Date:     2013-03-19
Description: 核算结果查询
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caaicx33_inq)
int f_caaicx33_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	CDbCommand cmd(conn);
	CString  sqlstr("");
	CString begin_time("");
	CString end_time("");
	CString mat_code("");
	CString sub_backlog_code("");
	CString shift_group("");
	CString sg_sign("");


	int i = 0;

	CModel tcaai04("TCAAI04");
	CDbCommand cmd_inq(conn);

	/* ***** 静态变量定义 ***** */
	int 	doFlag = 0;
	int 	fetchRowCount = 0;
	int     RowCount = 0;

	try
	{

		begin_time = bcls_rec->Tables[0].Rows[0]["BEGIN_TIME"].ToString();
		end_time = bcls_rec->Tables[0].Rows[0]["END_TIME"].ToString();
		tcaai04.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 通用
			sqlstr = " select t.*,t2.mat_name from ("
				" select dept_code,product_code mat_code,prod_shift_group,prod_shift_no,sub_Backlog_code,equ_no,sg_sign,mat_thick,vfree1,vfree2,vfree3,vfree4,vfree5,SUM(WT) wt,0 use_wt,0 cost_gj ,0 cost_cj ,0 cost_yj ,'产出' type"
				" from tcaai03"
				" WHERE 1=1"
				;
			if (begin_time.Trim() != "")  sqlstr = sqlstr + " AND  stats_period>= @begin_time";
			if (end_time.Trim() != "")  sqlstr = sqlstr + " AND  stats_period <= @end_time";
			if (tcaai04["DEPT_CODE"].ToString().Trim() != "") sqlstr = sqlstr + " AND  DEPT_CODE = @dept_code";
			sqlstr = sqlstr + " group by dept_code,product_code,prod_shift_group,prod_shift_no,sub_Backlog_code,equ_no,sg_sign,mat_thick,vfree1,vfree2,vfree3,vfree4,vfree5"
				" union all"
				" select dept_code, mat_code, prod_shift_group, prod_shift_no, sub_Backlog_code, equ_no, sg_sign, mat_thick, vfree1, vfree2, vfree3, vfree4, vfree5, 0 wt, sum(WT) use_wt, sum(cost_gj) cost_gj, sum(cost_cj) cost_cj,sum(cost_yj) cost_yj, '消耗' type"
				" from tcaai04"
				" WHERE 1=1"
				;
			if (begin_time.Trim() != "")  sqlstr = sqlstr + " AND  stats_period>= @begin_time";
			if (end_time.Trim() != "")  sqlstr = sqlstr + " AND  stats_period <= @end_time";
			if (tcaai04["DEPT_CODE"].ToString().Trim() != "") sqlstr = sqlstr + " AND  DEPT_CODE = @dept_code";
			sqlstr = sqlstr + " group by dept_code, mat_code, prod_shift_group, prod_shift_no, sub_Backlog_code, equ_no, sg_sign, mat_thick, vfree1, vfree2, vfree3, vfree4, vfree5) t"
				" left join tcaac11 t2 on decode (INSTR(t.mat_code,'_', 1, 1),0,t.mat_code,substr(t.mat_code,1,INSTR(t.mat_code,'_', 1, 1)-1)) = t2.mat_code"
				" WHERE 1=1"
				;
			if (tcaai04["MAT_NAME"].ToString().Trim() != "") sqlstr = sqlstr + " AND  instr(t2.MAT_NAME,@mat_name)>0  ";
			if (tcaai04["MAT_CODE"].ToString().Trim() != "")         sqlstr = sqlstr + " AND  t.MAT_CODE = @mat_code";
			if (tcaai04["SUB_BACKLOG_CODE"].ToString().Trim() != "") sqlstr = sqlstr + " AND  t.SUB_BACKLOG_CODE = @sub_backlog_code";
			if (tcaai04["SG_SIGN"].ToString().Trim() != "") sqlstr = sqlstr + " AND  t.SG_SIGN = @sg_sign";
			if (tcaai04["VFREE1"].ToString().Trim() != "") sqlstr = sqlstr + " AND  t.VFREE1 = @vfree1";
			if (tcaai04["VFREE2"].ToString().Trim() != "") sqlstr = sqlstr + " AND  t.VFREE2 = @vfree2";
			if (tcaai04["MAT_THICK"].ToDecimal() != 0) sqlstr = sqlstr + " AND  t.MAT_THICK = @mat_thick";
			if (tcaai04["PROD_SHIFT_GROUP"].ToString().Trim() != "") sqlstr = sqlstr + " AND  t.PROD_SHIFT_GROUP = @prod_shift_group";
			if (tcaai04["PROD_SHIFT_NO"].ToString().Trim() != "") sqlstr = sqlstr + " AND  t.PROD_SHIFT_NO = @prod_shift_no";
			if (tcaai04["EQU_NO"].ToString().Trim() != "") sqlstr = sqlstr + " AND  t.EQU_NO = @equ_no";
			sqlstr = sqlstr + " order BY  t.dept_code, t.sub_Backlog_code,t.equ_no,t.prod_shift_group,t.prod_shift_no,t.sg_sign,t.mat_thick,t.vfree1,t.vfree2,t.vfree3,t.vfree4,t.vfree5,t.mat_code ";
			break;
		}

		Log::Trace("", "", "sqlstr={0},", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("begin_time", begin_time.SubstringNE(0, 8));
		cmd_inq.Parameters.Set("end_time", end_time.SubstringNE(0, 8));
		cmd_inq.Parameters.Set("mat_code", tcaai04["MAT_CODE"].ToString());
		cmd_inq.Parameters.Set("sub_backlog_code", tcaai04["SUB_BACKLOG_CODE"].ToString());
		cmd_inq.Parameters.Set("dept_code", tcaai04["DEPT_CODE"].ToString());
		cmd_inq.Parameters.Set("sg_sign", tcaai04["SG_SIGN"].ToString());
		cmd_inq.Parameters.Set("vfree1", tcaai04["VFREE1"].ToString());
		cmd_inq.Parameters.Set("vfree2", tcaai04["VFREE2"].ToString());
		cmd_inq.Parameters.Set("mat_thick", tcaai04["MAT_THICK"].ToDecimal());
		cmd_inq.Parameters.Set("prod_shift_group", tcaai04["PROD_SHIFT_GROUP"].ToString());
		cmd_inq.Parameters.Set("prod_shift_no", tcaai04["PROD_SHIFT_NO"].ToString());
		cmd_inq.Parameters.Set("equ_no", tcaai04["EQU_NO"].ToString());
		cmd_inq.Parameters.Set("mat_name", tcaai04["MAT_NAME"].ToString());
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
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

