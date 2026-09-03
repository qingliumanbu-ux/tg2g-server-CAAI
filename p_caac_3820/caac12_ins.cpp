/*************************************************
Copyright:   Baosight Software LTD.co Copyright (c) 2010
Author:      zhouli
Version:     3.0
Date:        2011-39-14 09:39:03
Description: 物料价格新增
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//外部函数声明

/*<remark>=========================================================
/// <summary>
/// 物料价格新增
/// <para>
/// 1.根据传入的前台数据，新增数据进tcaac12表。
/// 2.写物料异动履历
/// </para>
/// <para>数据库表：TCAAC12(物料价格信息表)         </para>
/// <para>主调用函数：前台FormCAAC12画面(F3)调用。   </para>
/// </summary>
/// <param name="TCAAC12">整表数据    </param>
/// <returns>处理结果</returns>
===========================================================</remark>*/

// service入口
BM2F_ENTERACE(caac12_ins)

int f_caac12_ins(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

    /* 程序内部变量 */
	int   doFlag = 0;
    int   fetchRowCount = 0;
	int   outBlockRow=0;
	int   ren = 0; 
    int   v_count=0;
	int   blkNum = 0;
	CDecimal v_max_version_no = 0;
	CDecimal v_roll_wt=0;
	CDecimal v_roll_num=0;
	CDecimal v_bundle_wt=0;
	CDecimal v_out_mat_num=0;

    /* 业务变量 */
	CString  datetime("");

	/* 实体类定义 */
	CModel tcaac11("TCAAC11");
	CModel tcaac12("TCAAC12");

	// 数据库SQL操作字符串
	CString  sqlstr("");

	CDbCommand cmd_tcaac12_inq(conn);
	CDbCommand cmd_inq(conn);

    try
    { 
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
        //获得传入的数据行数
		int count = bcls_rec->Tables[0].Rows.get_Count();
		
		for (int i = 0; i <  count ; i++ )
		{
            //重置头文件,获得头文件中的默认值
			tcaac12.Reset();

			//取得一行数据， MergeFrom方法将获得bcls_rec指定表的指定行的数据，并赋值给CTCAAC12类型的对象
			tcaac12.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			if(tcaac12["PRICE_UNIT"].ToDecimal() == 0)
			{
				sprintf(s.msg,"价格为0!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			if(tcaac12["PRICE_TERMS"].ToString().Trim() == "")
			{
				sprintf(s.msg,"价格术语为空!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			if(tcaac12["MAT_CODE"].ToString().Trim() == "")
			{
				sprintf(s.msg,"产副品代码无值!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			if (tcaac12.QueryCount("MAT_CODE, VALID_TIME_START, PRICE_TERMS") != 0)
			{
				sprintf(s.msg, "物料代码为[%s]，生效日期为[%s],价格类型为[%s]已经存在!", (const char*)tcaac12["MAT_CODE"].ToString(), (const char*)tcaac12["VALID_TIME_START"].ToString(), (const char*)tcaac12["PRICE_TERMS"].ToString());
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			

			sqlstr = " select mat_name from tcaac11 "
				" where mat_code=@mat_code ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("mat_code", tcaac12["MAT_CODE"]);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				tcaac12["MAT_NAME"] = cmd_inq.GetString(1);
			}
			cmd_inq.Close();


			//switch(conn->DatabaseKind)
			//{
			//	case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			//	case DB_KIND_MSSQL:	        // MS SQL Server数据库
			//	case DB_KIND_ORACLE:	        // Oracle 数据库
			//		sqlstr = CString(" SELECT MAX(VERSION_NO) FROM tcaac12 "
			//					" WHERE  MAT_CODE = @tcaac12.MAT_CODE  AND PRICE_TERMS=@tcaac12.PRICE_TERMS and dept_code=@dept_code ");
			//	break;
			//}
			//
			//cmd_tcaac12_inq.SetCommandText(sqlstr);
			//cmd_tcaac12_inq.Parameters.Set("tcaac12.MAT_CODE",tcaac12["MAT_CODE"]);
			//cmd_tcaac12_inq.Parameters.Set("tcaac12.PRICE_TERMS",tcaac12["PRICE_TERMS"]);
			//cmd_tcaac12_inq.Parameters.Set("dept_code", tcaac12["DEPT_CODE"]);
			//cmd_tcaac12_inq.ExecuteReader();

			////循环从游标中取数据，压回前台
			//if(cmd_tcaac12_inq.Read())
			//{
			//	v_max_version_no =cmd_tcaac12_inq.GetDecimal(1);

			//	//打印数据
			//	EDLog(1,1,"v_max_version_no=%f",v_max_version_no.ToInt32());
			//}
			//else
			//{
			//	v_max_version_no = 0;
			//}
			//cmd_tcaac12_inq.Close();

			tcaac12["REC_CREATOR"]=s.userid;   //记录创建责任者
			tcaac12["REC_CREATE_TIME"]=datetime;   //记录创建时刻
			tcaac12["VERSION_NO"]=v_max_version_no + 1; //版本号
			tcaac12["VALID_FLAG"]="1";   //生效标志

            //新增一条记录
            tcaac12.Insert();

			////将其他的版本的价格设置为历史
			//sqlstr = "UPDATE tcaac12 set VALID_FLAG = '0' "
			//	"  WHERE VERSION_NO < @tcaac12.VERSION_NO"
			//	"  AND MAT_CODE = @tcaac12.MAT_CODE"
			//	"  and back_code_1=@back_code_1 "
			//	"  and dept_code=@dept_code "
			//	"  AND PRICE_TERMS = @tcaac12.PRICE_TERMS"
			//	;
			//cmd_inq.SetCommandText(sqlstr);
			//cmd_inq.Parameters.Set("tcaac12.MAT_CODE",tcaac12["MAT_CODE"]);
			//cmd_inq.Parameters.Set("tcaac12.PRICE_TERMS",tcaac12["PRICE_TERMS"]);
			//cmd_inq.Parameters.Set("tcaac12.VERSION_NO",tcaac12["VERSION_NO"]);
			//cmd_inq.Parameters.Set("back_code_1", tcaac12["BACK_CODE_1"]);
			//cmd_inq.Parameters.Set("dept_code", tcaac12["DEPT_CODE"]);
			//cmd_inq.ExecuteNonQuery();
			//cmd_inq.Close();
		}  
	}
	catch(CDbException& ex)				// 捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;		
	}
	catch(CApplicationException& ex)	// 捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	cmd_inq.Close();
	return doFlag;
}